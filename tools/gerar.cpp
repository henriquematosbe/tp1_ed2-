// Gera os tres arquivos (ascendente, descendente, aleatorio) de uma quantidade.
// Uso: ./gerar <quantidade>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "../src/arquivo.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2 || atoi(argv[1]) <= 0) {
        printf("Uso: gerar <quantidade>\n");
        return 1;
    }
    int quantidade = atoi(argv[1]);
    const char* nomes[3] = {"ascendente", "descendente", "aleatorio"};

    for (int situacao = 1; situacao <= 3; situacao++) {
        std::string nome = "dados_" + std::to_string(quantidade) + "_" +
                           std::to_string(situacao) + ".bin";
        if (ArquivoValido(nome, quantidade)) {
            printf("%s (%s): ja existe\n", nome.c_str(), nomes[situacao - 1]);
        } else if (GerarArquivo(nome, quantidade, situacao)) {
            printf("%s (%s): criado\n", nome.c_str(), nomes[situacao - 1]);
        } else {
            printf("Erro ao criar %s\n", nome.c_str());
            return 1;
        }
    }
    return 0;
}
