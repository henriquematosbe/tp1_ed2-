#ifndef ARVOREB_HPP
#define ARVOREB_HPP

#include <cstdio>        // FILE
#include "registro.hpp"  // Registro e Metricas

#define int M  = 2;        // ordem da arvore: cada pagina tem de M a 2M itens
#define int MM = 2 * M;    // maximo de itens em uma pagina

typedef struct {
    int Chave;       // chave de pesquisa
    int Posicao;     // numero do registro no arquivo (0, 1, 2, ...)
} TipoRegistro;

typedef struct TipoPagina* TipoApontador;

typedef struct TipoPagina {
    short n;                    // numero de itens na pagina
    TipoRegistro r[MM];         // itens, em ordem crescente de chave
    TipoApontador p[MM + 1];    // ponteiros para as paginas filhas
} TipoPagina;

// Le o arquivo inteiro e monta a arvore em memoria.
// Retorna false se der erro de leitura.
bool CriarArvoreB(FILE* arq, int quantidade, TipoApontador* Arvore, Metricas* m);

// Pesquisa a chave na arvore. Se encontrar, le o registro completo do arquivo, guarda em reg e retorna true.
bool PesquisarArvoreB(FILE* arq, TipoApontador Arvore, int chave, Registro* reg, Metricas* m);

// Libera toda a memoria usada pela arvore
void LiberarArvoreB(TipoApontador Ap);

#endif 
