#ifndef SEQUENCIAL_HPP
#define SEQUENCIAL_HPP

#include <vector>
#include "registro.hpp"

#define ITENSPAGINA 4

// Guarda no indice a chave do primeiro item de cada pagina.
// transf = leituras do arquivo, comp = comparacoes entre chaves
void CriarIndiceSequencial(const char* nomeArq, int quantidade,
                           std::vector<int>* indice, long* transf, long* comp);

// Pesquisa o item pela chave. Retorna 1 se encontrou (item preenchido)
int PesquisarSequencial(const char* nomeArq, int quantidade,
                        std::vector<int>* indice, tipoitem* item,
                        long* transf, long* comp);

#endif
