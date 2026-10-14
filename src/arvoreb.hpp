// A arvore fica na memoria e guarda so (chave, posicao do registro).
// O registro completo fica no arquivo e e lido no fim da pesquisa.

#ifndef ARVOREB_HPP
#define ARVOREB_HPP

#include <cstdio>        // FILE
#include "registro.hpp"  // Registro e Metricas

#define M  = 2;        // ordem: cada pagina tem de M a 2M itens
#define MM = 2 * M;    // maximo de itens em uma pagina

// Item guardado na arvore
struct TipoRegistro {
    int Chave;     // chave de pesquisa
    int Posicao;   // numero do registro no arquivo (0, 1, 2)
};

// Pagina da arvore: n itens e n+1 filhos
struct TipoPagina {
    int n;                       // quantos itens tem agora
    TipoRegistro r[MM];          // itens, em ordem crescente
    TipoPagina* p[MM + 1];       // filhos
};

typedef TipoPagina* TipoApontador;

// Le o arquivo inteiro e monta a arvore. Retorna false se der erro.
bool CriarArvoreB(FILE* arq, int quantidade, TipoApontador* arvore, Metricas* m);

// Procura a chave. Se achar, le o registro do arquivo em reg e retorna true.
bool PesquisarArvoreB(FILE* arq, TipoApontador arvore, int chave, Registro* reg, Metricas* m);

// Libera a memoria da arvore
void LiberarArvoreB(TipoApontador ap);

#endif
