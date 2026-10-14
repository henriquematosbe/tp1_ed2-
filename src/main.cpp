// main.cpp - Pesquisa externa: le os argumentos, garante o arquivo de dados,
// roda o metodo escolhido e mostra o resultado e as metricas.
//
// Uso: pesquisa <metodo> <quantidade> <situacao> <chave> [-P]

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#include "arquivo.hpp"
#include "registro.hpp"
#include "sequencial.hpp"
#include "arvorebin.hpp"
#include "arvoreb.hpp"
#include "arvorebest.hpp"

// Mostra como usar o programa
void ImprimirUso() {
    printf("Uso: pesquisa <metodo> <quantidade> <situacao> <chave> [-P]\n");
    printf("  metodo:   1 = sequencial indexado, 2 = arvore binaria, 3 = arvore B, 4 = arvore B*\n");
    printf("  situacao: 1 = ascendente, 2 = descendente, 3 = aleatorio\n");
}

// Mostra as metricas de uma etapa
void ImprimirMetricas(const char* etapa, Metricas m) {
    printf("%s_TRANSF=%ld %s_COMP=%ld %s_TEMPO=%.9f\n",
           etapa, m.transferencias, etapa, m.comparacoes, etapa, m.tempo);
}

int main(int argc, char* argv[]) {
    // ── 1. Argumentos ──
    if (argc < 5 || argc > 6) {
        ImprimirUso();
        return 1;
    }

    int metodo     = atoi(argv[1]);
    int quantidade = atoi(argv[2]);
    int situacao   = atoi(argv[3]);
    int chave      = atoi(argv[4]);
    bool mostrarChaves = (argc == 6);

    if (mostrarChaves && strcmp(argv[5], "-P") != 0) {
        printf("Erro: argumento opcional invalido (use -P).\n");
        return 1;
    }
    if (metodo < 1 || metodo > 4 || quantidade <= 0 || situacao < 1 || situacao > 3) {
        printf("Erro: metodo deve ser 1 a 4, quantidade maior que 0 e situacao 1 a 3.\n");
        ImprimirUso();
        return 1;
    }
    if (metodo == 1 && situacao != 1) {
        printf("Erro: o sequencial indexado exige arquivo ascendente (situacao 1).\n");
        return 1;
    }
    if (metodo == 2 && situacao != 3) {
        printf("Erro: a arvore binaria so se aplica a arquivo aleatorio (situacao 3).\n");
        return 1;
    }

    // ── 2. Arquivo de dados (criado aqui, fora do tempo medido) ──
    std::string nomeArq = "dados_" + std::to_string(quantidade) + "_" +
                          std::to_string(situacao) + ".bin";
    std::string nomeNos = "arvorebin_" + std::to_string(quantidade) + "_" +
                          std::to_string(situacao) + ".idx";

    if (!ArquivoValido(nomeArq, quantidade) && !GerarArquivo(nomeArq, quantidade, situacao)) {
        printf("Erro: nao foi possivel criar o arquivo %s.\n", nomeArq.c_str());
        return 1;
    }

    if (mostrarChaves) {
        printf("Chaves do arquivo:\n");
        ImprimirChaves(nomeArq, quantidade);
    }

    // Os metodos 3 e 4 recebem o arquivo ja aberto
    FILE* arq = nullptr;
    if (metodo == 3 || metodo == 4) {
        arq = fopen(nomeArq.c_str(), "rb");
        if (arq == nullptr) {
            printf("Erro: nao foi possivel abrir %s.\n", nomeArq.c_str());
            return 1;
        }
    }

    Registro x;
    x.chave = chave;
    bool achou = false;
    Metricas indice, pesquisa;      // metricas de cada etapa

    std::vector<int> tabela;        // indice do metodo 1
    TipoApontador raizB = nullptr;  // arvore do metodo 3
    ApontadorBE raizBE = nullptr;   // arvore do metodo 4

    // ── 3. Etapa 1: criacao do indice ──
    clock_t inicio = clock();
    if (metodo == 1) {
        CriarIndiceSequencial(nomeArq.c_str(), quantidade, &tabela,
                              &indice.transferencias, &indice.comparacoes);
    } else if (metodo == 2) {
        CriarArvoreBinaria(nomeArq.c_str(), nomeNos.c_str(), quantidade,
                           &indice.transferencias, &indice.comparacoes);
    } else if (metodo == 3) {
        if (!CriarArvoreB(arq, quantidade, &raizB, &indice)) {
            printf("Erro: falha ao ler o arquivo para criar a arvore B.\n");
            fclose(arq);
            return 1;
        }
    } else {
        CriarArvoreBEstrela(arq, quantidade, &raizBE, &indice);
    }
    indice.tempo = (double)(clock() - inicio) / CLOCKS_PER_SEC;

    // ── 4. Etapa 2: pesquisa ──
    inicio = clock();
    if (metodo == 1) {
        achou = PesquisarSequencial(nomeArq.c_str(), quantidade, &tabela, &x,
                                    &pesquisa.transferencias, &pesquisa.comparacoes);
    } else if (metodo == 2) {
        achou = PesquisarArvoreBinaria(nomeArq.c_str(), nomeNos.c_str(), &x,
                                       &pesquisa.transferencias, &pesquisa.comparacoes);
    } else if (metodo == 3) {
        achou = PesquisarArvoreB(arq, raizB, chave, &x, &pesquisa);
    } else {
        achou = PesquisarArvoreBEstrela(arq, raizBE, chave, &x, &pesquisa);
    }
    pesquisa.tempo = (double)(clock() - inicio) / CLOCKS_PER_SEC;

    // ── 5. Resultado ──
    if (achou) {
        printf("ENCONTRADA=1\n");
        printf("CHAVE=%d DADO1=%ld\n", x.chave, x.dado1);
        printf("DADO2=%.40s...\n", x.dado2);   // so o comeco: o campo tem 5000 chars
    } else {
        printf("ENCONTRADA=0\n");
    }
    ImprimirMetricas("INDICE", indice);
    ImprimirMetricas("PESQUISA", pesquisa);

    // ── 6. Libera a memoria e fecha o arquivo ──
    LiberarArvoreB(raizB);
    LiberarArvoreBEstrela(raizBE);
    if (arq != nullptr) fclose(arq);
    return 0;
}
