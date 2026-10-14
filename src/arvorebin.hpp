#ifndef ARVOREBIN_HPP
#define ARVOREBIN_HPP

#include "registro.hpp"

// Nodo da arvore (fica no arquivo de nos, nao o registro inteiro)
typedef struct {
    int chave;
    long posicao; // posicao do registro no arquivo de dados
    int esq;      // filho esquerdo no arquivo de nos (-1 = nulo)
    int dir;      // filho direito no arquivo de nos (-1 = nulo)
} tiponodo;

// Cria o arquivo de nos lendo o arquivo de dados em sequencia
void CriarArvoreBinaria(const char* nomeArq, const char* nomeNos, int quantidade,
                        long* transf, long* comp);

// Pesquisa o item pela chave. Retorna 1 se encontrou (item preenchido)
int PesquisarArvoreBinaria(const char* nomeArq, const char* nomeNos,
                           tipoitem* item, long* transf, long* comp);

#endif
