//gerar arquivo
#ifndef ARQUIVO_H
#define ARQUIVO_H

#include <string>

// Retorna true se o arquivo existe e tem o tamanho esperado
// (quantidade * sizeof(Registro)). Um arquivo incompleto conta como invalido.
bool ArquivoValido(const std::string& nome, int quantidade);

// Gera o arquivo binario com chaves de 1 a quantidade.
// situacao: 1 = ascendente, 2 = descendente, 3 = aleatorio
bool GerarArquivo(const std::string& nome, int quantidade, int situacao);

// Imprime na tela as chaves dos registros do arquivo (opcao -P)
bool ImprimirChaves(const std::string& nome, int quantidade);

#endif
