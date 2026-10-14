#include "arvoreb.hpp"

#include <cstdlib>   // malloc e free

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

// Posiciona o arquivo em 'desloc' bytes desde o inicio.
// Usa versoes de 64 bits porque o arquivo pode passar de 2 GB.
static int Seek64(FILE* arq, long long desloc) {
#ifdef _WIN32
    return _fseeki64(arq, desloc, SEEK_SET);       // Windows
#else
    return fseeko(arq, (off_t)desloc, SEEK_SET);   // Linux
#endif
}

static void Inicializa(TipoApontador* Arvore) {
    *Arvore = NULL;
}

//Faz a inserção daquele registro na pagina 
static void InsereNaPagina(TipoApontador Ap, TipoRegistro Reg, TipoApontador ApDir, Metricas* m){
    
    short NaoAchouPosicao;
    int k;
    k = Ap->n;
    NaoAchouPosicao = (k > 0);
    
    while (NaoAchouPosicao) {
        m->comparacoes++;
        if (Reg.Chave >= Ap->r[k - 1].Chave) {
            NaoAchouPosicao = FALSE;
            break;
        }
        //empurra pra proxima posição, caso a chave seja menor
        Ap->r[k] = Ap->r[k - 1];
        Ap->p[k + 1] = Ap->p[k];
        k--;
        if (k < 1) NaoAchouPosicao = FALSE;
    }
    //Adiciona a chave na pagina, caso aquela parte na pagina não tenha nada (NaoAchouPosicao==FALSO)
    Ap->r[k] = Reg;
    Ap->p[k + 1] = ApDir;
    Ap->n++;
}

//Faz o processo de recursão/conferencia (É chamada na função Insere)
static void Ins(TipoRegistro Reg, TipoApontador Ap, short* Cresceu, TipoRegistro* RegRetorno, TipoApontador* ApRetorno, Metricas* m){
    
    long i = 1;
    long j;
    TipoApontador ApTemp;

    if (Ap == NULL) {
        *Cresceu = TRUE;
        (*RegRetorno) = Reg; //reg que vai retornar 
        (*ApRetorno) = NULL; //pra onde essa reg (chave) vai apontar 
        return;
    }
    
    while (i < Ap->n) {
        m->comparacoes++;
        if (Reg.Chave > Ap->r[i - 1].Chave) i++;
        else break;
    }

    m->comparacoes++;
    if (Reg.Chave == Ap->r[i - 1].Chave) {
        printf(" Erro: Registro ja esta presente\n");
        *Cresceu = FALSE;
        return;
    }

    m->comparacoes++;
    if (Reg.Chave < Ap->r[i - 1].Chave) i--;
    Ins(Reg, Ap->p[i], Cresceu, RegRetorno, ApRetorno, m);
    
    if (!*Cresceu) return;
    if (Ap->n < MM) {   // Pagina tem espaco
        InsereNaPagina(Ap, *RegRetorno, *ApRetorno, m);
        *Cresceu = FALSE;
        return;
    }

    // Overflow: Pagina tem que ser dividida
    //Cria e inicializa o apontador temporario
    ApTemp = (TipoApontador)malloc(sizeof(TipoPagina));
    ApTemp->n = 0;
    ApTemp->p[0] = NULL;

    //Verifica se o valor vai entrar na pagina do apontador temporario ou na do "filho que estamos"
    if (i < M + 1){
        InsereNaPagina(ApTemp, Ap->r[MM - 1], Ap->p[MM], m);
        Ap->n--;
        InsereNaPagina(Ap, *RegRetorno, *ApRetorno, m);
    }else
        InsereNaPagina(ApTemp, *RegRetorno, *ApRetorno, m);
    
    for (j = M + 2; j <= MM; j++) 
        InsereNaPagina(ApTemp, Ap->r[j - 1], Ap->p[j], m);
    
    //Atualiza as informações da pagina depois do overflow
    //Principalmente pra onde cada coisa aponta  
    Ap->n = M;
    ApTemp->p[0] = Ap->p[M + 1];
    *RegRetorno = Ap->r[M];
    *ApRetorno = ApTemp;
}

static void Insere(TipoRegistro Reg, TipoApontador* Ap, Metricas* m){
    short Cresceu = FALSE;
    TipoRegistro RegRetorno;
    TipoApontador ApRetorno = NULL, ApTemp;

    Ins(Reg, *Ap, &Cresceu, &RegRetorno, &ApRetorno, m);

    if (Cresceu) {   // Arvore cresce na altura pela raiz
        ApTemp = (TipoApontador)malloc(sizeof(TipoPagina)); //Cria um apontador temporario 
        ApTemp->n = 1;
        ApTemp->r[0] = RegRetorno;
        ApTemp->p[1] = ApRetorno;
        ApTemp->p[0] = *Ap;
        *Ap = ApTemp;
    }
}

// Le o arquivo inteiro, registro por registro, e insere cada chave na arvore
bool CriarArvoreB(FILE* arq, int quantidade, TipoApontador* Arvore, Metricas* m){
    Registro reg;
    TipoRegistro item;

    Inicializa(Arvore);
    rewind(arq);

    for (int i = 0; i < quantidade; i++) {
        if (fread(&reg, sizeof(Registro), 1, arq) != 1) return false;
        m->transferencias++;      // 1 leitura de registro

        item.Chave = reg.chave;
        item.Posicao = i;         // posicao do registro no arquivo
        Insere(item, Arvore, m);
    }
    return true;
}

// Procura x->Chave a partir da pagina Ap. Se achar, *x recebe o item (com a Posicao do registro no arquivo).

static bool Pesquisa(TipoRegistro* x, TipoApontador Ap, Metricas* m){
    
    long i = 1;
    if (Ap == NULL) return false;  

    // Pesquisa sequencial para se encontrar o intervalo desejado
    while (i < Ap->n) {
        m->comparacoes++;
        if (x->Chave > Ap->r[i - 1].Chave) i++;
        else break;
    }
    // Verifica se a chave desejada foi localizada
    m->comparacoes++;
    if (x->Chave == Ap->r[i - 1].Chave) {
        *x = Ap->r[i - 1];
        return true;
    }
    // Ativacao recursiva da Pesquisa em uma das subarvores (esquerda ou direita)
    m->comparacoes++;
    if (x->Chave < Ap->r[i - 1].Chave) return Pesquisa(x, Ap->p[i - 1], m);
    else return Pesquisa(x, Ap->p[i], m);
}

bool PesquisarArvoreB(FILE* arq, TipoApontador Arvore, int chave, Registro* reg, Metricas* m){
    
    TipoRegistro x;
    x.Chave = chave;

    if (!Pesquisa(&x, Arvore, m)) return false;   // nao esta na arvore: nada e lido
    // Achou na arvore: le o registro completo do arquivo
    if (Seek64(arq, (long long)x.Posicao * sizeof(Registro)) != 0) return false;
    if (fread(reg, sizeof(Registro), 1, arq) != 1) return false;
    m->transferencias++;
    return true;
}

void LiberarArvoreB(TipoApontador Ap){
    if (Ap == NULL) return;
    for (int i = 0; i <= Ap->n; i++) LiberarArvoreB(Ap->p[i]);
    free(Ap);
}
