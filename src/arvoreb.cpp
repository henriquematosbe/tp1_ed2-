// arvoreb.cpp - Arvore B (Ziviani) para pesquisa externa.

#include "arvoreb.hpp"

// Pula para 'desloc' bytes do inicio do arquivo.
// Usa a versao de 64 bits porque o arquivo pode passar de 2 GB.
static int Seek64(FILE* arq, long long desloc) {
#ifdef _WIN32
    return _fseeki64(arq, desloc, SEEK_SET);
#else
    return fseeko(arq, desloc, SEEK_SET);
#endif
}

// Coloca o item 'reg' (e o filho 'dir' a direita dele) na pagina 'ap',
// mantendo a ordem. A pagina precisa ter espaco.
static void InsereNaPagina(TipoApontador ap, TipoRegistro reg, TipoApontador dir, Metricas* m) {
    int k = ap->n;

    // Empurra para a direita os itens maiores que reg
    while (k > 0) {
        m->comparacoes++;
        if (reg.Chave >= ap->r[k - 1].Chave) break;
        ap->r[k] = ap->r[k - 1];
        ap->p[k + 1] = ap->p[k];
        k--;
    }
    ap->r[k] = reg;
    ap->p[k + 1] = dir;
    ap->n++;
}

// Insere 'reg' na subarvore 'ap' (recursiva).
// Se a pagina dividir, devolve cresceu = true, o item que sobe (regSobe)
// e a nova pagina da direita (apSobe), para o pai encaixar.
static void Ins(TipoRegistro reg, TipoApontador ap, bool* cresceu,
                TipoRegistro* regSobe, TipoApontador* apSobe, Metricas* m) {
    // Chegou abaixo de uma folha: o item sobe para ser encaixado
    if (ap == nullptr) {
        *cresceu = true;
        *regSobe = reg;
        *apSobe = nullptr;
        return;
    }

    // Acha a posicao i do item na pagina
    int i = 1;
    while (i < ap->n) {
        m->comparacoes++;
        if (reg.Chave > ap->r[i - 1].Chave) i++;
        else break;
    }

    m->comparacoes++;
    if (reg.Chave == ap->r[i - 1].Chave) {   // chave repetida: nao insere
        *cresceu = false;
        return;
    }

    m->comparacoes++;
    if (reg.Chave < ap->r[i - 1].Chave) i--;

    // Desce para o filho certo
    Ins(reg, ap->p[i], cresceu, regSobe, apSobe, m);
    if (!*cresceu) return;

    // O filho dividiu: se ha espaco, so encaixa o item que subiu
    if (ap->n < MM) {
        InsereNaPagina(ap, *regSobe, *apSobe, m);
        *cresceu = false;
        return;
    }

    // Pagina cheia: divide em duas
    TipoApontador nova = new TipoPagina;
    nova->n = 0;
    nova->p[0] = nullptr;

    if (i < M + 1) {
        // o item novo fica na esquerda; o ultimo da esquerda vai para a nova
        InsereNaPagina(nova, ap->r[MM - 1], ap->p[MM], m);
        ap->n--;
        InsereNaPagina(ap, *regSobe, *apSobe, m);
    } else {
        // o item novo vai para a nova pagina
        InsereNaPagina(nova, *regSobe, *apSobe, m);
    }

    // A metade da direita vai para a nova pagina
    for (int j = M + 2; j <= MM; j++)
        InsereNaPagina(nova, ap->r[j - 1], ap->p[j], m);

    // O item do meio sobe para o pai
    ap->n = M;
    nova->p[0] = ap->p[M + 1];
    *regSobe = ap->r[M];
    *apSobe = nova;
}

// Insere um item na arvore. Se a raiz dividir, cria uma raiz nova
// (a arvore B cresce pela raiz).
static void Insere(TipoRegistro reg, TipoApontador* raiz, Metricas* m) {
    bool cresceu = false;
    TipoRegistro regSobe;
    TipoApontador apSobe = nullptr;

    Ins(reg, *raiz, &cresceu, &regSobe, &apSobe, m);

    if (cresceu) {
        TipoApontador nova = new TipoPagina;
        nova->n = 1;
        nova->r[0] = regSobe;
        nova->p[0] = *raiz;
        nova->p[1] = apSobe;
        *raiz = nova;
    }
}

bool CriarArvoreB(FILE* arq, int quantidade, TipoApontador* arvore, Metricas* m) {
    *arvore = nullptr;
    rewind(arq);

    Registro reg;
    for (int i = 0; i < quantidade; i++) {
        if (fread(&reg, sizeof(Registro), 1, arq) != 1) return false;
        m->transferencias++;              // 1 registro lido

        TipoRegistro item;
        item.Chave = reg.chave;
        item.Posicao = i;                 // posicao do registro no arquivo
        Insere(item, arvore, m);
    }
    return true;
}

// Procura x->Chave a partir da pagina ap (recursiva).
// Se achar, x recebe o item (com a posicao do registro).
static bool Pesquisa(TipoRegistro* x, TipoApontador ap, Metricas* m) {
    if (ap == nullptr) return false;

    // Acha o intervalo da chave na pagina
    int i = 1;
    while (i < ap->n) {
        m->comparacoes++;
        if (x->Chave > ap->r[i - 1].Chave) i++;
        else break;
    }

    m->comparacoes++;
    if (x->Chave == ap->r[i - 1].Chave) {   // achou
        *x = ap->r[i - 1];
        return true;
    }

    // Desce para o filho da esquerda ou da direita
    m->comparacoes++;
    if (x->Chave < ap->r[i - 1].Chave) return Pesquisa(x, ap->p[i - 1], m);
    return Pesquisa(x, ap->p[i], m);
}

bool PesquisarArvoreB(FILE* arq, TipoApontador arvore, int chave, Registro* reg, Metricas* m) {
    TipoRegistro x;
    x.Chave = chave;
    if (!Pesquisa(&x, arvore, m)) return false;   // nao esta na arvore

    // Achou: le o registro completo do arquivo
    if (Seek64(arq, (long long)x.Posicao * sizeof(Registro)) != 0) return false;
    if (fread(reg, sizeof(Registro), 1, arq) != 1) return false;
    m->transferencias++;
    return true;
}

void LiberarArvoreB(TipoApontador ap) {
    if (ap == nullptr) return;
    for (int i = 0; i <= ap->n; i++) LiberarArvoreB(ap->p[i]);
    delete ap;
}
