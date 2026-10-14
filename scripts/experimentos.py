# Roda os experimentos: para cada (metodo, quantidade, situacao) pesquisa 20 chaves
# bem espacadas (todas existem no arquivo) e grava a MEDIA das metricas em CSV.
# Uso: python3 scripts/experimentos.py 100 200 2000        (quantidades a testar)
import subprocess, sys, csv

quantidades = [int(q) for q in sys.argv[1:]] or [100, 200, 2000]
# situacoes que cada metodo aceita (1 asc, 2 desc, 3 aleatorio)
combos = [(1, 1), (2, 3), (3, 1), (3, 2), (3, 3)]
campos = ["INDICE_TRANSF", "INDICE_COMP", "INDICE_TEMPO",
          "PESQUISA_TRANSF", "PESQUISA_COMP", "PESQUISA_TEMPO"]

def ler(saida):
    valores = {}
    for linha in saida.split("\n"):
        for parte in linha.split(" "):
            if "=" in parte:
                nome, valor = parte.split("=", 1)
                if nome in campos:
                    valores[nome] = float(valor)
    return valores

with open("resultados/resultados.csv", "w", newline="") as f:
    saida_csv = csv.writer(f)
    saida_csv.writerow(["metodo", "quantidade", "situacao"] + campos)

    for n in quantidades:
        subprocess.run(["./gerar", str(n)], stdout=subprocess.DEVNULL, check=True)
        for metodo, situacao in combos:
            soma = {c: 0.0 for c in campos}
            for k in range(20):
                chave = 1 + k * (n - 1) // 19   # 20 chaves espalhadas de 1 ate n
                r = subprocess.run(["./pesquisa", str(metodo), str(n), str(situacao), str(chave)],
                                   capture_output=True, text=True)
                v = ler(r.stdout)
                for c in campos:
                    soma[c] += v[c]
            media = [soma[c] / 20 for c in campos]
            saida_csv.writerow([metodo, n, situacao] + media)
            print("metodo", metodo, "n", n, "situacao", situacao, "pronto")
print("Resultados em resultados/resultados.csv")
