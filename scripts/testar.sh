#!/bin/bash
# Teste de corretude: pesquisa TODAS as chaves e conta quantas foram encontradas.
# Uso: bash scripts/testar.sh <quantidade>      (rode na pasta do projeto)
N=${1:-100}
./gerar $N > /dev/null

# "metodo situacao": combinacoes que cada metodo aceita
for combo in "1 1" "2 3" "3 1" "3 2" "3 3"; do
    set -- $combo
    achou=0
    for k in $(seq 1 $N); do
        if ./pesquisa $1 $N $2 $k | grep -q "ENCONTRADA=1"; then achou=$((achou + 1)); fi
    done
    # chaves que NAO existem (0 e N+1) devem dar ENCONTRADA=0
    nao=0
    for k in 0 $((N + 1)); do
        if ./pesquisa $1 $N $2 $k | grep -q "ENCONTRADA=0"; then nao=$((nao + 1)); fi
    done
    if [ $achou -eq $N ] && [ $nao -eq 2 ]; then r="OK"; else r="ERRO"; fi
    echo "metodo $1 situacao $2: achou $achou/$N, inexistentes certos $nao/2  -> $r"
done
