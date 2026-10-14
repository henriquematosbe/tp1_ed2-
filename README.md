# tp1_ed2 - Trabalho Pratico I (Pesquisa Externa) - Estrutura de Dados II

Grupo: Caio, Daniel e Henrique

## Compilar
    make            # gera ./pesquisa e ./gerar

## Gerar os arquivos (ascendente, descendente e aleatorio) de uma quantidade
    ./gerar 2000    # cria dados_2000_1.bin, dados_2000_2.bin, dados_2000_3.bin

## Pesquisar
    ./pesquisa <metodo> <quantidade> <situacao> <chave> [-P]
    ./pesquisa 3 2000 3 1234

## Testar
    bash scripts/testar.sh 100                     # confere se acha todas as chaves
    python3 scripts/experimentos.py 100 200 2000   # media de 20 chaves -> resultados/resultados.csv
