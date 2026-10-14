#include "arvoreb.hpp"

#include <chrono>
#include <new>           // std::bad_alloc
#include <sys/types.h>   // off_t (usado pelo fseeko no Linux)

// Posiciona o arquivo em 'desloc' bytes desde o inicio.
// Usa versoes de 64 bits porque o arquivo pode passar de 2 GB.
static int Seek64(FILE* arq, long long desloc) {
#ifdef _WIN32
    return _fseeki64(arq, desloc, SEEK_SET);       // Windows
#else
    return fseeko(arq, (off_t)desloc, SEEK_SET);   // Linux
#endif
}

// Segundos decorridos desde 'inicio'
static double SegundosDesde(std::chrono::steady_clock::time_point inicio) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - inicio).count();
}

// Cria uma pagina vazia (sem itens e sem filhos)
static ApontadorB NovaPagina() {
    ApontadorB p = new PaginaB;
    p->n = 0;
    for (int i = 0; i <= MM_B; i++) p->filhos[i] = nullptr;
    return p;
}

// Insere 'item' na posicao 'i' da pagina 'ap' (que tem espaco), junto com o
// ponteiro 'apDir', que fica a direita do item.
static void InsereNaPagina(ApontadorB ap, int i, ItemB item, ApontadorB apDir) {
    
    for (int j = ap->n; j > i; j--) {          // abre espaco deslocando para a direita
        ap->itens[j]     = ap->itens[j - 1];
        ap->filhos[j + 1] = ap->filhos[j];
    }
    ap->itens[i]      = item;
    ap->filhos[i + 1] = apDir;
    ap->n++;
}

// Insere 'item' na subarvore de raiz 'ap'. Se a subarvore cresceu (a pagina
// dividiu), *cresceu = true e o item que deve subir fica em *itemRet, com a
// nova pagina irma (a direita) em *apRet. Descer ate nullptr equivale a
// "inserir abaixo de uma folha": o item sobe para a folha.
static void Ins(ItemB item, ApontadorB ap, bool* cresceu, ItemB* itemRet, ApontadorB* apRet, Metricas* m) {
    
  if (ap == nullptr) {
        *cresceu = true;
        *itemRet = item;
        *apRet = nullptr;
        return;
    }

    // Pesquisa sequencial na pagina: i = posicao do item (ou filho por onde
    // descer). Cada chave examinada conta como 1 comparacao.
    int i = 0;
    while (i < ap->n) {
        m->comparacoes++;
        if (item.chave == ap->itens[i].chave) {   // chave repetida: ignora
            *cresceu = false;
            return;
        }
        if (item.chave < ap->itens[i].chave) break;
        i++;
    }

    Ins(item, ap->filhos[i], cresceu, itemRet, apRet, m);
    if (!*cresceu) return;

    // O filho i dividiu (ou estamos numa folha): *itemRet entra na posicao i
    if (ap->n < MM_B) {                 // pagina tem espaco
        InsereNaPagina(ap, i, *itemRet, *apRet);
        *cresceu = false;
        return;
    }

    // Overflow: a pagina tem 2M itens e vai receber mais um. Monta vetores
    // temporarios com os 2M+1 itens (ja com o novo) e divide:
    // M itens ficam aqui, o item do meio sobe e os outros M vao para a irma.
    ItemB       tmpItens[MM_B + 1];
    ApontadorB  tmpFilhos[MM_B + 2];

    for (int j = 0; j < i; j++) tmpItens[j] = ap->itens[j];
    tmpItens[i] = *itemRet;
    for (int j = i; j < MM_B; j++) tmpItens[j + 1] = ap->itens[j];

    for (int j = 0; j <= i; j++) tmpFilhos[j] = ap->filhos[j];
    tmpFilhos[i + 1] = *apRet;
    for (int j = i + 1; j <= MM_B; j++) tmpFilhos[j + 1] = ap->filhos[j];

    ApontadorB irma = NovaPagina();

    ap->n = M_B;
    for (int j = 0; j < M_B; j++)  ap->itens[j]  = tmpItens[j];
    for (int j = 0; j <= M_B; j++) ap->filhos[j] = tmpFilhos[j];
    for (int j = M_B + 1; j <= MM_B; j++) ap->filhos[j] = nullptr;   // limpa restos

    irma->n = M_B;
    for (int j = 0; j < M_B; j++)  irma->itens[j]  = tmpItens[M_B + 1 + j];
    for (int j = 0; j <= M_B; j++) irma->filhos[j] = tmpFilhos[M_B + 1 + j];

    *itemRet = tmpItens[M_B];   // item do meio sobe para o pai
    *apRet   = irma;            // *cresceu continua true
}

// Insere na arvore inteira; se a raiz dividir, a arvore cresce em altura.
static void Insere(ItemB item, ApontadorB* raiz, Metricas* m) {
   
    bool cresceu = false;
    ItemB itemRet;
    ApontadorB apRet = nullptr;

    Ins(item, *raiz, &cresceu, &itemRet, &apRet, m);

    if (cresceu) {                      // nova raiz com um unico item
        ApontadorB nova = NovaPagina();
        nova->n = 1;
        nova->itens[0]  = itemRet;
        nova->filhos[0] = *raiz;
        nova->filhos[1] = apRet;
        *raiz = nova;
    }
}

bool CriarArvoreB(FILE* arq, int quantidade, ApontadorB* raiz, Metricas* m) {
    
    auto inicio = std::chrono::steady_clock::now();
    bool ok = true;

    *raiz = nullptr;                    // arvore vazia (Inicializa)

    try {
        // Le o arquivo inteiro, registro por registro, e insere cada chave
        if (Seek64(arq, 0) != 0) {
            ok = false;
        } else {
            Registro reg;
            for (int i = 0; i < quantidade && ok; i++) {
                if (std::fread(&reg, sizeof(Registro), 1, arq) != 1) {
                    ok = false;
                    break;
                }
                m->transferencias++;    // 1 leitura de registro

                ItemB item;
                item.chave   = reg.chave;
                item.posicao = i;       // posicao do registro no arquivo
                Insere(item, raiz, m);
            }
        }
    } catch (const std::bad_alloc&) {   // sem memoria para novas paginas
        ok = false;
    }

    m->tempo += SegundosDesde(inicio);
    return ok;
}

// Procura 'chave' a partir da pagina 'ap'. Se achar, guarda em *posicao a
// posicao do registro no arquivo.
static bool Pesquisa(ApontadorB ap, int chave, int* posicao, Metricas* m) {
    if (ap == nullptr) return false;    // chegou abaixo de uma folha: nao existe

    // Pesquisa sequencial para encontrar o intervalo desejado
    int i = 0;
    while (i < ap->n) {
        m->comparacoes++;
        if (chave == ap->itens[i].chave) {   // achou a chave
            *posicao = ap->itens[i].posicao;
            return true;
        }
        if (chave < ap->itens[i].chave) break;
        i++;
    }

    // Ativacao recursiva em uma das subarvores (esquerda de itens[i])
    return Pesquisa(ap->filhos[i], chave, posicao, m);
}

bool PesquisarArvoreB(FILE* arq, ApontadorB raiz, int chave, Registro* reg, Metricas* m) {
    
  auto inicio = std::chrono::steady_clock::now();
    bool achou = false;
    int posicao = 0;

    if (Pesquisa(raiz, chave, &posicao, m)) {
        // Achou na arvore: le o registro completo do arquivo
        long long desloc = (long long)posicao * sizeof(Registro);
        if (Seek64(arq, desloc) == 0 && std::fread(reg, sizeof(Registro), 1, arq) == 1) {
            m->transferencias++;
            achou = true;
        }
    }

    m->tempo += SegundosDesde(inicio);
    return achou;
}

void LiberarArvoreB(ApontadorB raiz) {
    if (raiz == nullptr) return;
    for (int i = 0; i <= raiz->n; i++) LiberarArvoreB(raiz->filhos[i]);
    delete raiz;
}
