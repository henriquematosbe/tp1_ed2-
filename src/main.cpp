#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>

#include "arquivo.h"
#include "registro.h"

// Mostra como o programa deve ser chamado
void ImprimirUso() {
    std::fprintf(stderr,
        "Uso: pesquisa <metodo> <quantidade> <situacao> <chave> [-P]\n"
        "  metodo:    1 = sequencial indexado, 2 = arvore binaria,\n"
        "             3 = arvore B, 4 = arvore B*\n"
        "  quantidade: quantidade de registros do arquivo (maior que 0)\n"
        "  situacao:  1 = ascendente, 2 = descendente, 3 = aleatorio\n"
        "  chave:     chave a ser pesquisada\n"
        "  -P:        imprime as chaves do arquivo na tela\n");
}

// Converte texto em inteiro. Retorna false se o texto nao for um numero.
bool LerInteiro(const char* texto, int& valor) {
    char* fim;
    long convertido = std::strtol(texto, &fim, 10);
    if (*texto == '\0' || *fim != '\0') return false;
    valor = (int)convertido;
    return true;
}

// Imprime as metricas de uma etapa em formato fixo (facil de ler por script)
void ImprimirMetricas(const char* etapa, const Metricas& m) {
    std::printf("%s_TRANSF=%ld %s_COMP=%ld %s_TEMPO=%.9f\n",
                etapa, m.transferencias, etapa, m.comparacoes, etapa, m.tempo);
}

int main(int argc, char* argv[]) {
    // ── 1. Validacao dos argumentos ──
    if (argc < 5 || argc > 6) {
        ImprimirUso();
        return 1;
    }

    int metodo, quantidade, situacao, chave;
    if (!LerInteiro(argv[1], metodo) || !LerInteiro(argv[2], quantidade) ||
        !LerInteiro(argv[3], situacao) || !LerInteiro(argv[4], chave)) {
        std::fprintf(stderr, "Erro: os quatro primeiros argumentos devem ser numeros inteiros.\n");
        ImprimirUso();
        return 1;
    }

    bool imprimirChaves = false;
    if (argc == 6) {
        if (std::strcmp(argv[5], "-P") != 0) {
            std::fprintf(stderr, "Erro: argumento opcional invalido (use -P).\n");
            return 1;
        }
        imprimirChaves = true;
    }

    if (metodo < 1 || metodo > 4) {
        std::fprintf(stderr, "Erro: metodo deve ser de 1 a 4.\n");
        return 1;
    }
    if (quantidade <= 0) {
        std::fprintf(stderr, "Erro: quantidade deve ser maior que 0.\n");
        return 1;
    }
    if (situacao < 1 || situacao > 3) {
        std::fprintf(stderr, "Erro: situacao deve ser 1, 2 ou 3.\n");
        return 1;
    }

    // ── 2. Situacoes de ordem que cada metodo aceita ──
    // (confirmar com o professor quais combinacoes valem)
    if (metodo == 1 && situacao != 1) {
        std::fprintf(stderr, "Erro: o sequencial indexado exige arquivo ascendente (situacao 1).\n");
        return 1;
    }
    if (metodo == 2 && situacao != 3) {
        std::fprintf(stderr, "Erro: a arvore binaria so se aplica a arquivo aleatorio (situacao 3).\n");
        return 1;
    }

    // ── 3. Garante que o arquivo de dados existe (fora do cronometro) ──
    std::filesystem::create_directories("dados");
    std::string nomeArq = "dados/dados_" + std::to_string(quantidade) + "_" +
                          std::to_string(situacao) + ".bin";

    if (!ArquivoValido(nomeArq, quantidade)) {
        if (!GerarArquivo(nomeArq, quantidade, situacao)) {
            std::fprintf(stderr, "Erro: nao foi possivel criar o arquivo %s.\n", nomeArq.c_str());
            return 1;
        }
    }

    // ── 4. Opcao -P: mostra as chaves do arquivo ──
    if (imprimirChaves) {
        std::printf("Chaves do arquivo:\n");
        ImprimirChaves(nomeArq, quantidade);
    }

    // ── 5. Executa o metodo escolhido ──
    Metricas metricasIndice;
    Metricas metricasPesquisa;
    Registro reg;
    bool encontrado = false;

    switch (metodo) {
        case 1:
            // TODO: CriarIndiceSequencial(...) e PesquisarSequencial(...)
            break;
        case 2:
            // TODO: CriarArvoreBinaria(...) e PesquisarArvoreBinaria(...)
            break;
        case 3:
            // TODO: CriarArvoreB(...) e PesquisarArvoreB(...)
            break;
        case 4:
            // TODO: CriarArvoreBEstrela(...) e PesquisarArvoreBEstrela(...)
            break;
    }

    // ── 6. Resultado ──
    if (encontrado) {
        std::printf("ENCONTRADA=1\n");
        std::printf("CHAVE=%d DADO1=%ld\n", reg.chave, reg.dado1);
        std::printf("DADO2=%s\n", reg.dado2);
    } else {
        std::printf("ENCONTRADA=0\n");
    }
    ImprimirMetricas("INDICE", metricasIndice);
    ImprimirMetricas("PESQUISA", metricasPesquisa);

    return 0;
}