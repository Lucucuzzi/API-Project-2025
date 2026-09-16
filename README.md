# Movhex — Prova Finale di Algoritmi e Strutture Dati (2024/2025)

Progetto della **Prova Finale di Algoritmi e Principi dell'Informatica (API)** del Politecnico di Milano, A.A. 2024/2025.

**Valutazione ottenuta: 30 e lode** 🎓

## Il problema

Movhex è una compagnia di autotrasporti che vuole calcolare le rotte ottimali per i suoi mezzi.
La mappa è una griglia rettangolare di **esagoni**: ogni esagono ha un costo di uscita (0–100, dove 0 = intransitabile)
e può avere fino a **5 rotte aeree** monodirezionali verso altri esagoni.

Il programma legge comandi da `stdin` e risponde su `stdout`:

| Comando | Descrizione |
|---|---|
| `init <colonne> <righe>` | Crea (o ricrea) la mappa con tutti i costi a 1 |
| `change_cost <x> <y> <v> <raggio>` | Modifica i costi degli esagoni entro `raggio` da `(x,y)`, in modo proporzionale alla distanza |
| `toggle_air_route <x1> <y1> <x2> <y2>` | Aggiunge o rimuove una rotta aerea da `(x1,y1)` a `(x2,y2)` |
| `travel_cost <xp> <yp> <xd> <yd>` | Stampa il costo minimo per andare da partenza a destinazione (`-1` se irraggiungibile) |

## Valutazione

Il progetto è valutato automaticamente da un verificatore online che misura **correttezza, tempo di esecuzione e memoria**
su 6 batterie di test, ognuna associata a un voto: 18, 21, 24, 27, 30, 30L.
Per ottenere un voto bisogna superare la batteria corrispondente **restando sotto i limiti di tempo e memoria** stabiliti.

Questa soluzione supera **tutte le batterie, inclusa quella del 30 e lode**, i cui limiti sono:

| Risorsa | Limite (30L) |
|---|---|
| Tempo di esecuzione | < 10 s |
| Memoria | < 15.5 MB |

Vincoli di realizzazione: C11, solo libreria standard, nessun multithreading.

## Scelte implementative

- **Mappa come array 1D** (`y * colonne + x`) invece di una matrice, per accessi in memoria contigui e una sola `malloc`.
- **`travel_cost` → Dijkstra con min-heap binario**, preallocato una volta per mappa (niente allocazioni durante le query).
  La ricerca si ferma appena viene estratta la destinazione.
- **`change_cost` → BFS** a partire dall'esagono centrale, limitata al raggio, per calcolare `DistEsagoni` sulla griglia esagonale
  (i vicini dipendono dalla parità della riga).
- **Rotte aeree** come lista concatenata per esagono (max 5 elementi).
- **Cache dei percorsi**: tabella hash da 8192 elementi con indirizzamento aperto e *doppio hashing*.
  Dato che la maggior parte dei comandi sono `travel_cost` concentrati sulle stesse zone, le risposte vengono memorizzate
  e la cache viene invalidata solo quando la mappa cambia (`init`, `change_cost`, `toggle_air_route`).
- Profilazione fatta con **Valgrind / Callgrind** per individuare i colli di bottiglia.

## Compilazione ed esecuzione

```bash
gcc -Wall -Werror -std=gnu11 -O2 progetto.c -o progetto -lm
./progetto < test_pubblici/example.txt
```

Verifica con i test pubblici forniti:

```bash
for t in test_pubblici/*.txt; do
  ./progetto < "$t" | diff -q - "$t.result" > /dev/null && echo "OK   $t" || echo "FAIL $t"
done
```

## Struttura

```
progetto.c       # soluzione finale (30 e lode)
Makefile         # flag di compilazione usati dal verificatore
test_pubblici/   # input e output attesi forniti dai docenti
```
