//coloquei seufixo para nao dar conflito com as variaveis do daniel (que vai fazer avoreB)
#ifndef ARVOREBEST_HPP
#define ARVOREBEST_HPP

#include <cstdio>       // FILE
#include "registro.hpp"   // Registro e Metricas

constexpr int M_BE  = 2;          // ordem da arvore B*
constexpr int MM_BE = 2 * M_BE;   // maximo de chaves/itens em uma pagina

// Item guardado nas folhas: a chave e onde o registro esta no arquivo
typedef struct {
    int chave;     // chave de pesquisa
    int posicao;   // numero do registro no arquivo (0, 1, 2, ...)
} ItemFolhaBE;

// Indica se a pagina e interna (indice) ou externa (folha)
typedef enum { InternaBE, ExternaBE } TipoPaginaBE;

typedef struct PaginaBE* ApontadorBE;   // ponteiro para uma pagina

typedef struct PaginaBE {
    TipoPaginaBE tipo;                    // InternaBE ou ExternaBE

    // Uma pagina e interna OU externa: os dois formatos dividem a memoria
    union {
        struct {                          // usado quando tipo == InternaBE
            int n;                        // quantas chaves tem agora
            int chaves[MM_BE];            // as "placas" de navegacao
            ApontadorBE filhos[MM_BE + 1];// n chaves -> n+1 filhos
        } interna;

        struct {                          // usado quando tipo == ExternaBE
            int n;                        // quantos itens tem agora
            ItemFolhaBE itens[MM_BE];     // os itens de verdade
        } externa;
    } u;
} PaginaBE;

// Le o arquivo e monta a arvore B* em memoria (etapa de criacao do indice).
// Soma em m as transferencias, as comparacoes e o tempo desta etapa.
void CriarArvoreBEstrela(FILE* arq, int quantidade, ApontadorBE* raiz, Metricas* m);

// Pesquisa a chave na arvore. Se encontrar, le o registro do arquivo,
// guarda em reg e retorna true. Soma em m as metricas da pesquisa.
bool PesquisarArvoreBEstrela(FILE* arq, ApontadorBE raiz, int chave,
                             Registro* reg, Metricas* m);

// Libera toda a memoria usada pela arvore
void LiberarArvoreBEstrela(ApontadorBE raiz);

#endif // ARVOREBEST_HPP