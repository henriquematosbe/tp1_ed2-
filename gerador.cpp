#include <cstdio>
#include <vector>
#include <algorithm>
#include <random>
#include <numeric>
#include "registro.h"

// Gera o arquivo binario com a quantidade e a situacao pedidas
// situacao: 1 = ascendente, 2 = descendente, 3 = aleatorio
bool GerarArquivo(const char* nome, int quantidade, int situacao) {
    // Chaves unicas de 1 a quantidade
    std::vector<int> chaves(quantidade);
    std::iota(chaves.begin(), chaves.end(), 1);

    if (situacao == 2) {
        std::reverse(chaves.begin(), chaves.end());
    } else if (situacao == 3) {
        std::mt19937 gerador(42); // semente fixa para repetir os testes
        std::shuffle(chaves.begin(), chaves.end(), gerador);
    }

    FILE* arq = std::fopen(nome, "wb");
    if (arq == nullptr) return false;

    Registro reg;
    for (int i = 0; i < quantidade; i++) {
        reg.chave = chaves[i];
        reg.dado1 = (long)chaves[i] * 10;
        std::fill(reg.dado2, reg.dado2 + 5000, 'a' + (chaves[i] % 26));
        reg.dado2[4999] = '\0';
        std::fwrite(&reg, sizeof(Registro), 1, arq);
    }

    std::fclose(arq);
    return true;
}