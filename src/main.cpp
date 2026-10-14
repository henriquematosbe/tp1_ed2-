#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>
#include <vector>
#include "arquivo.hpp"
#include "registro.hpp"
#include "sequencial.hpp"
#include "arvorebin.hpp"
#include "arvoreb.hpp"

int main(int argc, char* argv[]) {
    // Uso: pesquisa <metodo> <quantidade> <situacao> <chave> [-P]
    if (argc < 5 || argc > 6) {
        printf("Uso: pesquisa <metodo> <quantidade> <situacao> <chave> [-P]\n");
        printf("  metodo: 1 = sequencial indexado, 2 = arvore binaria, 3 = arvore B, 4 = arvore B*\n");
        printf("  situacao: 1 = ascendente, 2 = descendente, 3 = aleatorio\n");
        return 1;
    }

    int metodo = atoi(argv[1]);
    int quantidade = atoi(argv[2]);
    int situacao = atoi(argv[3]);
    int chave = atoi(argv[4]);
    int mostrarChaves = (argc == 6 && strcmp(argv[5], "-P") == 0);

    if (argc == 6 && !mostrarChaves) {
        printf("Erro: argumento opcional invalido (use -P).\n");
        return 1;
    }
    if (metodo < 1 || metodo > 4 || quantidade <= 0 || situacao < 1 || situacao > 3) {
        printf("Erro: metodo deve ser 1 a 4, quantidade maior que 0 e situacao 1 a 3.\n");
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
    if (metodo == 4) {
        printf("Metodo 4 (arvore B*) ainda nao implementado.\n");
        return 1;
    }

    // Garante que o arquivo de dados existe (fora do tempo medido)
    std::string nomeArq = "dados_" + std::to_string(quantidade) + "_" +
                          std::to_string(situacao) + ".bin";
    if (!ArquivoValido(nomeArq, quantidade)) {
        if (!GerarArquivo(nomeArq, quantidade, situacao)) {
            printf("Erro: nao foi possivel criar o arquivo %s.\n", nomeArq.c_str());
            return 1;
        }
    }

    if (mostrarChaves) {
        printf("Chaves do arquivo:\n");
        ImprimirChaves(nomeArq, quantidade);
    }

    Registro x;
    x.chave = chave;
    bool achou = false;
    Metricas indice, pesquisa; // metricas das duas etapas
    clock_t inicio;

    std::string nomeNos = "arvorebin_" + std::to_string(quantidade) + "_" +
                          std::to_string(situacao) + ".idx";
    std::vector<int> tabela;   // indice do metodo 1
    ApontadorB raiz = NULL;    // arvore do metodo 3
    FILE* arq = NULL;          // arquivo de dados (metodo 3)

    if (metodo == 3) {
        arq = fopen(nomeArq.c_str(), "rb");
        if (arq == NULL) {
            printf("Erro: nao foi possivel abrir %s.\n", nomeArq.c_str());
            return 1;
        }
    }

    // ── Etapa 1: criacao do indice ──
    inicio = clock();
    if (metodo == 1)
        CriarIndiceSequencial(nomeArq.c_str(), quantidade, &tabela,
                              &indice.transferencias, &indice.comparacoes);
    else if (metodo == 2)
        CriarArvoreBinaria(nomeArq.c_str(), nomeNos.c_str(), quantidade,
                           &indice.transferencias, &indice.comparacoes);
    else if (!CriarArvoreB(arq, quantidade, &raiz, &indice)) {
        printf("Erro: falha ao criar a arvore B.\n");
        return 1;
    }
    // O tempo e medido aqui para todos os metodos (mesmo criterio)
    indice.tempo = (double)(clock() - inicio) / CLOCKS_PER_SEC;
    printf("INDICE_TRANSF=%ld INDICE_COMP=%ld INDICE_TEMPO=%.9f\n",
           indice.transferencias, indice.comparacoes, indice.tempo);

    // ── Etapa 2: pesquisa ──
    inicio = clock();
    if (metodo == 1)
        achou = PesquisarSequencial(nomeArq.c_str(), quantidade, &tabela, &x,
                                    &pesquisa.transferencias, &pesquisa.comparacoes);
    else if (metodo == 2)
        achou = PesquisarArvoreBinaria(nomeArq.c_str(), nomeNos.c_str(), &x,
                                       &pesquisa.transferencias, &pesquisa.comparacoes);
    else
        achou = PesquisarArvoreB(arq, raiz, chave, &x, &pesquisa);
    pesquisa.tempo = (double)(clock() - inicio) / CLOCKS_PER_SEC;
    printf("PESQUISA_TRANSF=%ld PESQUISA_COMP=%ld PESQUISA_TEMPO=%.9f\n",
           pesquisa.transferencias, pesquisa.comparacoes, pesquisa.tempo);

    // ── Resultado ──
    if (achou) {
        printf("ENCONTRADA=1\n");
        printf("CHAVE=%d DADO1=%ld\n", x.chave, x.dado1);
        printf("DADO2=%.40s...\n", x.dado2); // so o comeco, o campo tem 5000 chars
    } else {
        printf("ENCONTRADA=0\n");
    }

    if (metodo == 3) {
        LiberarArvoreB(raiz);
        fclose(arq);
    }
    return 0;
}
