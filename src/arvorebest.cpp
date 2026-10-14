#include "arvorebest.hpp"

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