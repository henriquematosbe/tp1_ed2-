// arvorebest.cpp
// Implementacao da arvore B* para pesquisa externa.

#include "arvorebest.hpp"

#include <sys/types.h>   // off_t (usado pelo fseeko no Linux)

// -Funcoes auxiliares 

// Posiciona o arquivo em 'desloc' bytes desde o inicio.
// Usa versoes de 64 bits porque o arquivo pode passar de 2 GB.
static int Seek64(FILE* arq, long long desloc) {
#ifdef _WIN32
    return _fseeki64(arq, desloc, SEEK_SET);       // Windows
#else
    return fseeko(arq, (off_t)desloc, SEEK_SET);   // Linux
#endif
}

// Cria uma folha vazia
static ApontadorBE NovaFolha() {
    ApontadorBE p = new PaginaBE;
    p->tipo = ExternaBE;
    p->u.externa.n = 0;
    return p;
}

// Cria uma pagina interna vazia
static ApontadorBE NovaInterna() {
    ApontadorBE p = new PaginaBE;
    p->tipo = InternaBE;
    p->u.interna.n = 0;
    return p;
}

// ───────────────────── Pesquisa ─────────────────────

bool PesquisarArvoreBEstrela(FILE* arq, ApontadorBE raiz, int chave,
                             Registro* reg, Metricas* m) {
    if (raiz == nullptr) return false;  // arvore vazia

    // 1. Desce pelas paginas internas ate chegar numa folha
    ApontadorBE p = raiz;
    while (p->tipo == InternaBE) {
        int i = 0;
        while (i < p->u.interna.n) {
            m->comparacoes++;
            if (chave < p->u.interna.chaves[i]) break;
            i++;
        }
        p = p->u.interna.filhos[i];
    }

    // 2. Procura a chave dentro da folha
    for (int i = 0; i < p->u.externa.n; i++) {
        m->comparacoes++;
        if (p->u.externa.itens[i].chave == chave) {
            // 3. Achou: le o registro completo do arquivo
            long long desloc =
                (long long)p->u.externa.itens[i].posicao * sizeof(Registro);
            if (Seek64(arq, desloc) != 0) return false;
            if (fread(reg, sizeof(Registro), 1, arq) != 1) return false;
            m->transferencias++;
            return true;
        }
    }
    return false;   // a chave nao esta no arquivo
}

// Insercao 

// Insere 'item' na subarvore 'p' (funcao recursiva).
// Se a pagina 'p' encher e dividir, avisa quem chamou:
//   cresceu   = true
//   chaveSobe = chave que deve entrar na pagina de cima
//   novaDir   = nova pagina criada a direita de 'p'
static void Ins(ItemFolhaBE item, ApontadorBE p, bool* cresceu,
                int* chaveSobe, ApontadorBE* novaDir, Metricas* m) {
    *cresceu = false;

    if (p->tipo == ExternaBE) {
        // ── Caso 1: p e uma folha ──
        int n = p->u.externa.n;

        // Acha a posicao do item (a folha fica sempre ordenada).
        // As chaves do arquivo sao unicas, entao nao ha repetidas.
        int pos = 0;
        while (pos < n) {
            m->comparacoes++;
            if (item.chave < p->u.externa.itens[pos].chave) break;
            pos++;
        }

        // Folha com espaco: so encaixa o item na posicao certa
        if (n < MM_BE) {
            for (int j = n; j > pos; j--)
                p->u.externa.itens[j] = p->u.externa.itens[j - 1];
            p->u.externa.itens[pos] = item;
            p->u.externa.n++;
            return;
        }

        // Folha cheia: junta os MM_B E itens + o novo (MM_BE + 1, ordenados)
        ItemFolhaBE temp[MM_BE + 1];
        for (int j = 0, k = 0; j <= MM_BE; j++) {
            if (j == pos) temp[j] = item;
            else temp[j] = p->u.externa.itens[k++];
        }

        // Divide: M_BE itens ficam em p, M_BE + 1 vao para a nova folha
        ApontadorBE dir = NovaFolha();
        p->u.externa.n = 0;
        for (int j = 0; j < M_BE; j++)
            p->u.externa.itens[p->u.externa.n++] = temp[j];
        for (int j = M_BE; j <= MM_BE; j++)
            dir->u.externa.itens[dir->u.externa.n++] = temp[j];
 
        // A primeira chave da nova folha e  COPIADA para cima
        // (ela continua na folha, porque os itens so ficam nas folhas)
        *cresceu = true;
        *chaveSobe = dir->u.externa.itens[0].chave;
        *novaDir = dir;
        return;
    }

    // ── Caso 2  : p e uma pagina interna ──
    int n = p->u.interna.n;

    // Escolhe o filho (mesma regra da pesquisa)
    int i = 0;
    while (i < n) {
        m->comparacoes++;
        if (item.chave < p->u.interna.chaves[i]) break;
        i++;
    }

    // Insere no filho escolhido
    bool filhoCresceu;
    int chaveFilho;
    ApontadorBE dirFilho;
    Ins(item, p->u.interna.filhos[i], &filhoCresceu, &chaveFilho, &dirFilho, m);
    if (!filhoCresceu) return;   // o filho nao dividiu: nada a fazer aqui

    // O filho dividiu: chaveFilho entra na posicao i e dirFilho em filhos[i+1]

    // Pagina com espaco: so encaixa
    if (n < MM_BE) {
        for (int j = n; j > i; j--) {
            p->u.interna.chaves[j] = p->u.interna.chaves[j - 1];
            p->u.interna.filhos[j + 1] = p->u.interna.filhos[j];
        }
        p->u.interna.chaves[i] = chaveFilho;
        p->u.interna.filhos[i + 1] = dirFilho;
        p->u.interna.n++;
        return;
    }

    // Pagina interna cheia: junta t  udo em vetores temporarios
    int tempChaves[MM_BE + 1];
    ApontadorBE tempFilhos[MM_BE + 2];
    for (int j = 0, k = 0; j <= MM_BE; j++) {
        if (j == i) tempChaves[j] = chaveFilho;
        else tempChaves[j] = p->u.interna.chaves[k++];
    }
    for (int j = 0, k = 0; j <= MM_BE + 1; j++) {
        if (j == i + 1) tempFilhos[j] = dirFilho;
        else tempFilhos[j] = p->u.interna.filhos[k++];
    }

    // Divide: M_B  E chaves ficam em p, a do   meio SOBE, M_BE vao para a direita
    ApontadorBE dir = NovaInterna();
    p->u.interna.n = M_BE;
    for (int j = 0; j < M_BE; j++) {
        p->u.interna.chaves[j] = tempChaves[j];
        p->u.interna.filhos[j] = tempFilhos[j];
    }
    p->u.interna.filhos[M_BE] = tempFilhos[M_BE];

    dir->u.interna.n = M_BE;
    for (int j = 0; j < M_BE; j++) {
        dir->u.interna.chaves[j] = tempChaves[M_BE + 1 + j];
        dir->u.interna.filhos[j] = tempFilhos[M_BE + 1 + j];
    }
    dir->u.interna.filhos[M_BE] = tempFilhos[MM_BE + 1];

    // Em pagina interna a chave do meio   SOBE (nao fica copiada embaixo)
    *cresceu = true;
    *chaveSobe = tempChaves[M_BE];
    *novaDir = dir;
}

// Insere um item na arvore.    Se a raiz dividir, cria uma raiz nova
// e a arvore cresce um nivel (a arvore B cresce pela raiz).
static void Inserir(ItemFolhaBE item, ApontadorBE* raiz, Metricas* m) {
    if (*raiz == nullptr) {          // arvore vazia: a raiz e uma folha
        *raiz = NovaFolha();
        (*raiz)->u.externa.itens[0] = item;
        (*raiz)->u.externa.n = 1;
        return;
    }

    bool cresceu;
    int chaveSobe;
    ApontadorBE novaDir;
    Ins(item, *raiz, &cresceu, &chaveSobe, &novaDir, m);

    if (cresceu) {
        ApontadorBE novaRaiz = NovaInterna();
        novaRaiz->u.interna.n = 1;
        novaRaiz->u.interna.chaves[0] = chaveSobe;
        novaRaiz->u.interna.filhos[0] = *raiz;    // raiz antiga a esquerda
        novaRaiz->u.interna.filhos[1] = novaDir;  // pagina nova a direita
        *raiz = novaRaiz;
    }
}

//Criacao do indice 

void CriarArvoreBEstrela(FILE* arq, int quantidade, ApontadorBE* raiz, Metricas* m) {
    *raiz = nullptr;
    if (Seek64(arq, 0) != 0) return;   // volta ao inicio do arquivo

    Registro reg;
    for (int i = 0; i < quantidade; i++) {
        if (fread(&reg, sizeof(Registro), 1, arq) != 1) break;
        m->transferencias++;           // 1 registro lido do disco

        ItemFolhaBE item;
        item.chave = reg.chave;
        item.posicao = i;              // i-esimo registro do arquivo
        Inserir(item, raiz, m);
    }
}

// Liberacao

void LiberarArvoreBEstrela(ApontadorBE raiz) {
    if (raiz == nullptr) return;
    if (raiz->tipo == InternaBE) {
        for (int i = 0; i <= raiz->u.interna.n; i++)
            LiberarArvoreBEstrela(raiz->u.interna.filhos[i]);
    }
    delete raiz;
}