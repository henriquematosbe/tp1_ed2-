// experimentos.cpp - Gera os resultados do relatorio.
// Para cada quantidade e situacao: gera o arquivo (se preciso) e, para cada
// metodo que aceita a situacao, cria o indice uma vez e pesquisa 20 chaves
// espalhadas (1 + k*(n-1)/19), gravando a media em resultados/resultados.csv.
//
// Uso: experimentos                  (100, 200, 2000, 20000 e 200000 registros)
//      experimentos 2000000          (so as quantidades informadas)

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>
#include "../src/arquivo.hpp"
#include "../src/sequencial.hpp"
#include "../src/arvorebin.hpp"
#include "../src/arvoreb.hpp"
#include "../src/arvorebest.hpp"

// Segundos desde 'inicio' (relogio de alta resolucao)
double Segundos(std::chrono::steady_clock::time_point inicio) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - inicio).count();
}

// Roda um metodo: cria o indice, faz as 20 pesquisas e grava uma linha no CSV
void Rodar(FILE* csv, int m, int n, int sit, std::string arq) {
    FILE* f = fopen(arq.c_str(), "rb");
    Metricas idx;
    std::vector<int> tabela;
    TipoApontador raizB = nullptr;
    ApontadorBE raizBE = nullptr;

    // Cria o indice uma vez
    auto inicio = std::chrono::steady_clock::now();
    if (m == 1) CriarIndiceSequencial(arq.c_str(), n, &tabela, &idx.transferencias, &idx.comparacoes);
    if (m == 2) CriarArvoreBinaria(arq.c_str(), "nos.idx", n, &idx.transferencias, &idx.comparacoes);
    if (m == 3) CriarArvoreB(f, n, &raizB, &idx);
    if (m == 4) CriarArvoreBEstrela(f, n, &raizBE, &idx);
    idx.tempo = Segundos(inicio);

    // 20 pesquisas, somando as metricas de todas
    Metricas soma;
    int achadas = 0;
    for (int k = 0; k < 20; k++) {
        Registro x;
        x.chave = 1 + (int)((long long)k * (n - 1) / 19);
        bool achou = false;
        inicio = std::chrono::steady_clock::now();
        if (m == 1) achou = PesquisarSequencial(arq.c_str(), n, &tabela, &x, &soma.transferencias, &soma.comparacoes);
        if (m == 2) achou = PesquisarArvoreBinaria(arq.c_str(), "nos.idx", &x, &soma.transferencias, &soma.comparacoes);
        if (m == 3) achou = PesquisarArvoreB(f, raizB, x.chave, &x, &soma);
        if (m == 4) achou = PesquisarArvoreBEstrela(f, raizBE, x.chave, &x, &soma);
        soma.tempo += Segundos(inicio);
        if (achou && x.dado1 == x.chave * 10L) achadas++;   // registro certo?
    }

    fprintf(csv, "%d,%d,%d,%ld,%ld,%.6f,%.2f,%.2f,%.9f,%d\n", m, n, sit,
            idx.transferencias, idx.comparacoes, idx.tempo,
            soma.transferencias / 20.0, soma.comparacoes / 20.0, soma.tempo / 20, achadas);
    fflush(csv);
    printf("metodo %d, %d registros, situacao %d: %d/20 chaves achadas\n", m, n, sit, achadas);

    LiberarArvoreB(raizB);
    LiberarArvoreBEstrela(raizBE);
    fclose(f);
    std::filesystem::remove("nos.idx");
}

int main(int argc, char* argv[]) {
    std::vector<int> quantidades = {100, 200, 2000, 20000, 200000};
    if (argc > 1) {
        quantidades.clear();
        for (int i = 1; i < argc; i++) quantidades.push_back(atoi(argv[i]));
    }

    std::filesystem::create_directories("resultados");
    FILE* csv = fopen("resultados/resultados.csv", "w");
    fprintf(csv, "metodo,quantidade,situacao,indice_transf,indice_comp,indice_tempo_s,"
                 "pesquisa_transf_media,pesquisa_comp_media,pesquisa_tempo_medio_s,chaves_achadas\n");

    for (int n : quantidades) {
        for (int sit = 1; sit <= 3; sit++) {
            std::string arq = "dados_" + std::to_string(n) + "_" + std::to_string(sit) + ".bin";
            if (!ArquivoValido(arq, n)) GerarArquivo(arq, n, sit);

            if (sit == 1) Rodar(csv, 1, n, sit, arq);   // sequencial: so crescente
            if (sit == 3) Rodar(csv, 2, n, sit, arq);   // binaria: so aleatorio
            Rodar(csv, 3, n, sit, arq);
            Rodar(csv, 4, n, sit, arq);

            if (n >= 2000000) std::filesystem::remove(arq);   // arquivo de ~10 GB
        }
    }
    fclose(csv);
    printf("Resultados em resultados/resultados.csv\n");
    return 0;
}
