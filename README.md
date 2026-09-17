# Movhex - Prova Finale di API 🗺️

![C](https://img.shields.io/badge/c11-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![GCC](https://img.shields.io/badge/gcc-%23A42E2B.svg?style=for-the-badge&logo=gnu&logoColor=white)
![Make](https://img.shields.io/badge/make-%23427819.svg?style=for-the-badge&logo=gnu&logoColor=white)
![Valgrind](https://img.shields.io/badge/valgrind-%23333333.svg?style=for-the-badge&logoColor=white)
![Grade](https://img.shields.io/badge/Final_Grade-30L%2F30-brightgreen?style=for-the-badge)

**Politecnico di Milano - Anno Accademico 2024/2025**

Soluzione della **Prova Finale di Algoritmi e Principi dell'Informatica (API)**: calcolo di rotte ottimali su una mappa a esagoni con costi variabili e rotte aeree. Il progetto ha ottenuto il voto massimo di **30 e lode**.

---

## 📋 Indice
1. [Il problema](#-il-problema)
2. [Valutazione](#-valutazione)
3. [Scelte implementative](#-scelte-implementative)
4. [Compilazione ed esecuzione](#-compilazione-ed-esecuzione)

---

## 🧩 Il problema
La mappa è una griglia rettangolare di **esagoni**: ogni esagono ha un costo di uscita (0–100, 0 = intransitabile) e fino a **5 rotte aeree** monodirezionali. Il programma legge comandi da `stdin` e risponde su `stdout`:

| Comando | Descrizione |
|---|---|
| `init <colonne> <righe>` | Crea (o ricrea) la mappa con tutti i costi a 1 |
| `change_cost <x> <y> <v> <raggio>` | Modifica i costi entro `raggio` da `(x,y)`, in proporzione alla distanza |
| `toggle_air_route <x1> <y1> <x2> <y2>` | Aggiunge o rimuove una rotta aerea |
| `travel_cost <xp> <yp> <xd> <yd>` | Costo minimo tra partenza e destinazione (`-1` se irraggiungibile) |

---

## 🏆 Valutazione
Un verificatore automatico misura **correttezza, tempo e memoria** su 6 batterie di test (18 → 30L). Questa soluzione le supera tutte:

| Risorsa | Limite (30L) |
|---|---|
| Tempo | < 10 s |
| Memoria | < 15.5 MB |

Vincoli: C11, solo libreria standard, niente multithreading.

---

## ⚙️ Scelte implementative
* **Mappa come array 1D** (`y * colonne + x`): una sola `malloc`, accessi contigui.
* **`travel_cost` → Dijkstra con min-heap binario** preallocato per mappa, con uscita anticipata all'estrazione della destinazione.
* **`change_cost` → BFS** limitata al raggio (vicini dipendenti dalla parità della riga).
* **Cache dei percorsi**: hash table da 8192 elementi con *doppio hashing*, invalidata solo quando la mappa cambia.
* Profilazione con **Valgrind / Callgrind**.

---

## 🚀 Compilazione ed esecuzione
```bash
gcc -Wall -Werror -std=gnu11 -O2 progetto.c -o progetto -lm
./progetto < test_pubblici/example.txt
```

Test pubblici:
```bash
for t in test_pubblici/*.txt; do
  ./progetto < "$t" | diff -q - "$t.result" > /dev/null && echo "OK   $t" || echo "FAIL $t"
done
```
