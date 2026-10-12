#include "arquivo.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <numeric>
#include <random>
#include <vector>

#include "registro.h"

bool ArquivoValido(const std::string& nome, int quantidade) {
    std::error_code erro;
    if (!std::filesystem::exists(nome, erro)) return false;

    // O tamanho deve bater com a quantidade pedida
    unsigned long long esperado =
        (unsigned long long)quantidade * sizeof(Registro);
    return std::filesystem::file_size(nome, erro) == esperado;
}

bool GerarArquivo(const std::string& nome, int quantidade, int situacao) {
    // 1. Monta as chaves em memoria (1 a quantidade)
    std::vector<int> chaves(quantidade);
    std::iota(chaves.begin(), chaves.end(), 1);

    // 2. Aplica a situacao de ordem
    if (situacao == 2) {
        std::reverse(chaves.begin(), chaves.end());
    } else if (situacao == 3) {
        std::mt19937 gerador(42); // semente fixa: testes repetiveis
        std::shuffle(chaves.begin(), chaves.end(), gerador);
    }

    // 3. Grava em arquivo temporario; so vira o arquivo final no fim,
    // assim uma geracao interrompida nao deixa arquivo quebrado
    std::string temporario = nome + ".tmp";
    FILE* arq = std::fopen(temporario.c_str(), "wb");
    if (arq == nullptr) return false;

    Registro reg;
    for (int i = 0; i < quantidade; i++) {
        reg.chave = chaves[i];
        reg.dado1 = (long)chaves[i] * 10;
        std::fill(reg.dado2, reg.dado2 + 5000, (char)('a' + chaves[i] % 26));
        reg.dado2[4999] = '\0';

        if (std::fwrite(&reg, sizeof(Registro), 1, arq) != 1) {
            std::fclose(arq);
            return false;
        }
    }
    std::fclose(arq);

    std::error_code erro;
    std::filesystem::rename(temporario, nome, erro);
    return !erro;
}

bool ImprimirChaves(const std::string& nome, int quantidade) {
    FILE* arq = std::fopen(nome.c_str(), "rb");
    if (arq == nullptr) return false;

    Registro reg;
    for (int i = 0; i < quantidade; i++) {
        if (std::fread(&reg, sizeof(Registro), 1, arq) != 1) break;
        std::printf("%d\n", reg.chave);
    }

    std::fclose(arq);
    return true;
}