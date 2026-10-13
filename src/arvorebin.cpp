#include <stdio.h>
#include "arvorebin.hpp"

// Le o nodo da posicao pos do arquivo de nos
void LerNodo(FILE* arq, int pos, tiponodo* nodo) {
    fseek(arq, (long)pos * sizeof(tiponodo), SEEK_SET);
    fread(nodo, sizeof(tiponodo), 1, arq);
}

// Grava o nodo na posicao pos do arquivo de nos
void GravarNodo(FILE* arq, int pos, tiponodo* nodo) {
    fseek(arq, (long)pos * sizeof(tiponodo), SEEK_SET);
    fwrite(nodo, sizeof(tiponodo), 1, arq);
}

void CriarArvoreBinaria(const char* nomeArq, const char* nomeNos, int quantidade,
                        long* transf, long* comp) {
    FILE* dados = fopen(nomeArq, "rb");
    FILE* nos = fopen(nomeNos, "w+b"); // abre para ler e escrever
    tipoitem x;
    tiponodo novo, atual;
    int totalNos = 0;

    for (int i = 0; i < quantidade; i++) {
        // Le o proximo item do arquivo de dados
        fread(&x, sizeof(tipoitem), 1, dados);
        (*transf)++;

        novo.chave = x.chave;
        novo.posicao = i;
        novo.esq = -1;
        novo.dir = -1;

        // O primeiro item vira a raiz
        if (totalNos == 0) {
            GravarNodo(nos, 0, &novo);
            totalNos = 1;
            continue;
        }

        // Desce a arvore ate achar o pai do novo nodo
        int pos = 0;
        while (1) {
            LerNodo(nos, pos, &atual);
            (*transf)++;
            (*comp)++;
            if (x.chave < atual.chave) {
                if (atual.esq == -1) { atual.esq = totalNos; break; }
                pos = atual.esq;
            } else {
                if (atual.dir == -1) { atual.dir = totalNos; break; }
                pos = atual.dir;
            }
        }

        // Atualiza o pai e grava o novo nodo no fim do arquivo
        GravarNodo(nos, pos, &atual);
        GravarNodo(nos, totalNos, &novo);
        totalNos++;
    }

    fclose(dados);
    fclose(nos);
}

int PesquisarArvoreBinaria(const char* nomeArq, const char* nomeNos,
                           tipoitem* item, long* transf, long* comp) {
    FILE* dados = fopen(nomeArq, "rb");
    FILE* nos = fopen(nomeNos, "rb");
    tiponodo nodo;
    int pos = 0; // comeca pela raiz
    int achou = 0;

    while (pos != -1 && !achou) {
        LerNodo(nos, pos, &nodo);
        (*transf)++;
        (*comp)++;
        if (item->chave == nodo.chave) {
            // Achou: le o item completo no arquivo de dados
            fseek(dados, nodo.posicao * sizeof(tipoitem), SEEK_SET);
            fread(item, sizeof(tipoitem), 1, dados);
            (*transf)++;
            achou = 1;
        } else {
            (*comp)++;
            if (item->chave < nodo.chave) pos = nodo.esq;
            else pos = nodo.dir;
        }
    }

    fclose(dados);
    fclose(nos);
    return achou;
}