# Piano di Implementazione: Advanced Social Seating Logic

Questo documento descrive la strategia per implementare una gestione realistica dei tavoli nella simulazione della mensa, passando da un contatore globale di posti a un'architettura topologica basata su entità tavolo singole.

## 1. Modifica del Data Model (`include/common.h`)

La gestione passerà da un singolo semaforo a un array di strutture dati in memoria condivisa (SHM).

- **Definizione Struttura `Table`**:
  ```c
  typedef struct {
      int id;
      int capacity;       // 2, 4, o 6 posti
      int occupied_seats; // Posti attualmente occupati
  } Table;
  ```
- **Struttura `MainSharedMemory`**:
  - `Table tables[MAX_TABLE_ENTRIES]`: Array delle entità tavolo.
  - `int active_tables_count`: Numero di tavoli generati.
- **Semafori**:
  - `MUTEX_TABLE_SCAN`: Semaforo binario per proteggere l'accesso all'array durante la ricerca/modifica dei posti.
  - `SEM_TABLE_CONDITION`: Semaforo di segnalazione per l'attesa passiva (sostituisce l'attesa su semafori vuoti).

## 2. Inizializzazione e Setup (`setup_ipc.c` / `setup_population.c`)

Il Master deve configurare l'ambiente all'avvio:

- **Distribuzione**: Generare un numero di tavoli la cui somma delle capacità sia uguale a `NOFTABLESEATS`.
- **Varietà**: Distribuire le capacità (es: 30% tavoli da 2, 50% da 4, 20% da 6) per riflettere una mensa reale.
- **Stato Iniziale**: Impostare `occupied_seats = 0` per ogni tavolo.

## 3. Logica di Occupazione "Social" (`utente.c`)

L'utente (o il Leader del gruppo) non eseguirà più una semplice `reserve_sem`, ma un algoritmo di ricerca:

- **Algoritmo First-Fit**:
  1.  Acquisizione di `MUTEX_TABLE_SCAN`.
  2.  Ciclo sull'array `tables`.
  3.  Verifica: `if ((capacity - occupied_seats) >= group_size)`.
  4.  Esito Positivo: Incremento dei posti occupati, memorizzazione di `tavolo_id` nel processo utente, rilascio Mutex.
  5.  Esito Negativo: Rilascio Mutex e attesa bloccante su `SEM_TABLE_CONDITION`.

## 4. Logica di Rilascio e Notifica (`utente.c`)

Quando un gruppo finisce di mangiare:

- **De-allocazione**: Acquisizione Mutex, decremento `occupied_seats` del tavolo specifico, rilascio Mutex.
- **Broadcast**: Il processo deve inviare un segnale (una "V" sul semaforo di condizione o una serie di incrementi) per svegliare i gruppi in attesa, forzandoli a ripetere la scansione.

## 5. Sfide Tecniche e Benefici

- **Frammentazione**: Come nella RAM, potremmo avere 10 posti liberi totali ma nessun tavolo da 4 per un gruppo di amici. Questo aumenta il realismo della simulazione.
- **Concorrenza**: L'accesso atomico all'array dei tavoli previene le race condition dove due gruppi occupano lo stesso spazio.
- **Voto**: Questa implementazione eleva la complessità del progetto ben oltre la versione minima, dimostrando eccellenti capacità di design di sistemi concorrenti.
