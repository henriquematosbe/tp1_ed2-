#include <stdio.h>
#include "sequencial.hpp"

void CriarIndiceSequencial(const char* nomeArq, int quantidade,
                           std::vector<int>* indice, long* transf, long* comp) {
    FILE* arq = fopen(nomeArq, "rb");
    tipoitem x;
    int numPaginas = (quantidade + ITENSPAGINA - 1) / ITENSPAGINA;

    for (int p = 0; p < numPaginas; p++) {
        // Le o primeiro item da pagina p
        fseek(arq, (long)p * ITENSPAGINA * sizeof(tipoitem), SEEK_SET);
        fread(&x, sizeof(tipoitem), 1, arq);
        (*transf)++;
        indice->push_back(x.chave);
    }

    fclose(arq);
}

int PesquisarSequencial(const char* nomeArq, int quantidade, std::vector<int>* indice, tipoitem* item, long* transf, long* comp) {
    // Procura a pagina onde o item pode estar
    int tam = indice->size();
    int i = 0;
    while (i < tam) {
        (*comp)++;
        if ((*indice)[i] > item->chave) break;
        i++;
    }

    // Chave menor que a primeira chave: o item nao existe
    if (i == 0) return 0;

    // A ultima pagina pode ter menos itens
    int primeiro = (i - 1) * ITENSPAGINA;
    int quantitens = quantidade - primeiro;
    if (quantitens > ITENSPAGINA) quantitens = ITENSPAGINA;

    // Le a pagina desejada
    FILE* arq = fopen(nomeArq, "rb");
    tipoitem pagina[ITENSPAGINA];
    fseek(arq, (long)primeiro * sizeof(tipoitem), SEEK_SET);
    fread(pagina, sizeof(tipoitem), quantitens, arq);
    (*transf)++;
    fclose(arq);

    // Pesquisa sequencial dentro da pagina
    for (i = 0; i < quantitens; i++) {
        (*comp)++;
        if (pagina[i].chave == item->chave) {
            *item = pagina[i];
            return 1;
        }
    }
    return 0;
}