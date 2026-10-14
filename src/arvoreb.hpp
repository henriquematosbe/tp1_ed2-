#ifndef ARVOREB_HPP
#define ARVOREB_HPP

#include <cstdio>       
#include "registro.hpp"  // Registro e Metricas

#define int M_B  = 2;          // ordem da arvore B (use o mesmo valor da B*)
#define int MM_B = 2 * M_B;    // maximo de itens em uma pagina

// Item guardado nas paginas: a chave e onde o registro esta no arquivo
typedef struct {
    int chave;     // chave de pesquisa
    int posicao;   // numero do registro no arquivo (0, 1, 2, ...)
} ItemB;

typedef struct PaginaB* ApontadorB;   // ponteiro para uma pagina

typedef struct PaginaB {
    int n;                         // quantos itens a pagina tem agora
    ItemB itens[MM_B];             // itens em ordem crescente de chave
    ApontadorB filhos[MM_B + 1];   // n itens -> n+1 filhos (todos nullptr em folha)
} PaginaB;

// Le o arquivo e monta a arvore B em memoria
// Retorna false se houver erro de leitura; nesse caso a arvore
bool CriarArvoreB(FILE* arq, int quantidade, ApontadorB* raiz, Metricas* m);

// Pesquisa a chave na arvore. Se encontrar, le o registro do arquivo, guarda em reg e retorna true. 
bool PesquisarArvoreB(FILE* arq, ApontadorB raiz, int chave, Registro* reg, Metricas* m);

// Libera toda a memoria usada pela arvore
void LiberarArvoreB(ApontadorB raiz);

#endif
