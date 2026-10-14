// experimentos.cpp - Gera os resultados usados no relatorio.
//
// Para um tamanho de arquivo (n) e uma situacao de ordem, roda cada metodo
// que aceita essa situacao: cria o indice UMA vez e pesquisa 20 chaves
// espalhadas pelo arquivo (1 + k*(n-1)/19, k = 0..19), tirando a media.
// Cada linha e acrescentada em resultados/experimentos.csv.
//
// Uso: experimentos <quantidade> <situacao>
//      situacao: 1 = crescente, 2 = decrescente, 3 = aleatoria

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include "../src/arquivo.hpp"
#include "../src/registro.hpp"
#include "../src/sequencial.hpp"
#include "../src/arvorebin.hpp"
#include "../src/arvoreb.hpp"
#include "../src/arvorebest.hpp"

typedef std::chrono::steady_clock Relogio;

// Segundos entre dois instantes (relogio de alta resolucao)
double Segundos(Relogio::time_point a, Relogio::time_point b) {
    return std::chrono::duration<double>(b - a).count();
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Uso: experimentos <quantidade> <situacao>\n");
        return 1;
    }
    int n = atoi(argv[1]);
    int situacao = atoi(argv[2]);
    if (n <= 0 || situacao < 1 || situacao > 3) {
        printf("Erro: quantidade > 0 e situacao de 1 a 3.\n");
        return 1;
    }

    std::string nomeArq = "dados_" + std::to_string(n) + "_" + std::to_string(situacao) + ".bin";
    std::string nomeNos = "nos_" + std::to_string(n) + ".idx";
    if (!ArquivoValido(nomeArq, n) && !GerarArquivo(nomeArq, n, situacao)) {
        printf("Erro: nao foi possivel criar %s.\n", nomeArq.c_str());
        return 1;
    }

    // Metodos que aceitam cada situacao (igual ao main.cpp)
    std::vector<int> metodos;
    if (situacao == 1) metodos = {1, 3, 4};
    else if (situacao == 2) metodos = {3, 4};
    else metodos = {2, 3, 4};

    // Abre o CSV de resultados (escreve o cabecalho se ele for novo)
    std::filesystem::create_directories("resultados");
    std::string nomeCsv = "resultados/experimentos.csv";
    bool novo = !std::filesystem::exists(nomeCsv);
    FILE* csv = fopen(nomeCsv.c_str(), "a");
    if (csv == nullptr) {
        printf("Erro: nao foi possivel abrir %s.\n", nomeCsv.c_str());
        return 1;
    }
    if (novo)
        fprintf(csv, "metodo,quantidade,situacao,indice_transf,indice_comp,indice_tempo_s,"
                     "pesquisa_transf_media,pesquisa_comp_media,pesquisa_tempo_medio_s,chaves_achadas\n");

    for (int metodo : metodos) {
        Metricas indice;
        std::vector<int> tabela;          // metodo 1
        TipoApontador raizB = nullptr;    // metodo 3
        ApontadorBE raizBE = nullptr;     // metodo 4
        FILE* arq = fopen(nomeArq.c_str(), "rb");
        if (arq == nullptr) {
            printf("Erro: nao foi possivel abrir %s.\n", nomeArq.c_str());
            return 1;
        }

        // ── Criacao do indice (uma vez) ──
        Relogio::time_point inicio = Relogio::now();
        if (metodo == 1)
            CriarIndiceSequencial(nomeArq.c_str(), n, &tabela, &indice.transferencias, &indice.comparacoes);
        else if (metodo == 2)
            CriarArvoreBinaria(nomeArq.c_str(), nomeNos.c_str(), n, &indice.transferencias, &indice.comparacoes);
        else if (metodo == 3)
            CriarArvoreB(arq, n, &raizB, &indice);
        else
            CriarArvoreBEstrela(arq, n, &raizBE, &indice);
        indice.tempo = Segundos(inicio, Relogio::now());

        // ── 20 pesquisas: soma tudo e depois divide por 20 ──
        double somaTransf = 0, somaComp = 0, somaTempo = 0;
        int achadas = 0;
        for (int k = 0; k < 20; k++) {
            int chave = 1 + (int)((long long)k * (n - 1) / 19);
            Registro x;
            x.chave = chave;
            Metricas p;
            bool achou;

            Relogio::time_point a = Relogio::now();
            if (metodo == 1)
                achou = PesquisarSequencial(nomeArq.c_str(), n, &tabela, &x, &p.transferencias, &p.comparacoes);
            else if (metodo == 2)
                achou = PesquisarArvoreBinaria(nomeArq.c_str(), nomeNos.c_str(), &x, &p.transferencias, &p.comparacoes);
            else if (metodo == 3)
                achou = PesquisarArvoreB(arq, raizB, chave, &x, &p);
            else
                achou = PesquisarArvoreBEstrela(arq, raizBE, chave, &x, &p);
            somaTempo += Segundos(a, Relogio::now());

            // Confere se o registro devolvido e o certo (dado1 = chave * 10)
            if (achou && x.chave == chave && x.dado1 == (long)chave * 10) achadas++;
            somaTransf += p.transferencias;
            somaComp += p.comparacoes;
        }

        fprintf(csv, "%d,%d,%d,%ld,%ld,%.6f,%.2f,%.2f,%.9f,%d\n", metodo, n, situacao,
                indice.transferencias, indice.comparacoes, indice.tempo,
                somaTransf / 20, somaComp / 20, somaTempo / 20, achadas);
        printf("metodo %d, %d registros, situacao %d: %d/20 chaves achadas\n", metodo, n, situacao, achadas);

        LiberarArvoreB(raizB);
        LiberarArvoreBEstrela(raizBE);
        fclose(arq);
    }
    fclose(csv);

    // Apaga os arquivos auxiliares; o de 2 milhoes tem ~10 GB
    std::filesystem::remove(nomeNos);
    if (n >= 2000000) std::filesystem::remove(nomeArq);
    return 0;
}
