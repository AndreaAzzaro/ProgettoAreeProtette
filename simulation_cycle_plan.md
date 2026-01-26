# Progetto Mensa Oasi: Piano d'Azione Raffinato

Questo documento definisce l'architettura finale per la sincronizzazione e la comunicazione tra i processi della simulazione.

## 1. Protocollo di Sincronizzazione "Bidirezionale"

Per evitare race conditions e garantire che nessun processo (incluso il Master) proceda prematuramente, utilizziamo un set di semafori per ogni barriera (Startup, Mattina, Sera).

### Struttura Barriera (READY, GATE)

1. **READY (Index N):** Inizializzato al numero di partecipanti (Master + Figli). Ogni processo che arriva chiama `P(READY)` (decrementa).
2. **GATE (Index N+1):** Inizializzato a 1 (Chiuso). I figli chiamano `wait_for_zero(GATE)` per passare.

### Sequenza d'Esecuzione

1. **Fase Preparatoria (Master):**
   - Calcola `total_p = 1 (Master) + NOF_WORKERS + NOF_CASHIERS + NOF_USERS`.
   - Setup Barriera: `READY = total_p`, `GATE = 1`.
2. **Arrivo alla Barriera (Figli):**
   - Chiamano `sync_child_start`: Decrementano `READY` e aspettano che `GATE` diventi 0.
3. **Sblocco Barriera (Master):**
   - Decrementa `READY` (segnala la propria presenza).
   - Chiama `wait_for_zero(READY)` per assicurarsi che l'intero gruppo sia pronto.
   - Chiama `open_barrier_gate(GATE)` (imposta `GATE` a 0) per liberare tutti i figli.

---

## 2. Ciclo di Vita delle Entità

### Master (Simulation Engine)

- **Bootstrap:** Crea IPC, lancia figli, sincronizza startup.
- **Loop Giorno:**
  1. Configura Barriera Mattina.
  2. Apre Barriera Mattina.
  3. Prepara Barriera Sera (mentre si lavora).
  4. Avvia Timer POSIX (SIGRTMIN) per Refill e Fine Giornata.
  5. `sigsuspend()`: Attesa passiva guidata da eventi.
  6. Fine Giornata: Chiama Barriera Sera.
  7. Calcola statistiche e verifica Overload.
- **Cleanup:** SIGTERM ai PGID dei figli, rimozione IPC.

### Utente

- **Loop Vita:** Attesa Barriera Mattina -> Attività Mensa -> Attesa Barriera Sera.
- **Attività Mensa:**
  1. Scelta piatti.
  2. `P(Posto_al_bancone)`.
  3. `msgsnd(Ordine)` -> `msgrcv(ACK, PID)`.
  4. `V(Posto_al_bancone)`.
  5. `msgsnd(Pagamento)`.
  6. `P(Posto_a_sedere)` -> Mangia -> `V(Posto_a_sedere)`.

### Operatore (Cibo/Cassa)

- **Loop Vita:** Attesa Barriera Mattina -> Lavoro -> Attesa Barriera Sera.
- **Lavoro:** `msgrcv(Ordine)` -> `nanosleep(servizio)` -> `msgsnd(ACK, user_pid)`.

---

## 3. Matrice Comunicazione IPC

| Canale          | Meccanismo         | Mittente  | Destinatario   | Messaggio                 |
| :-------------- | :----------------- | :-------- | :------------- | :------------------------ |
| Distribuzione   | Code Messaggi (x3) | Utente    | Operatore      | `OrderPayload` (T: 1)     |
| Feedback Cibo   | Coda Stazione      | Operatore | Utente         | `ACK` (T: PID_UTENTE)     |
| Pagamento       | Coda Cassa         | Utente    | Cassiere       | `PaymentPayload` (T: 1)   |
| Posti / Bancone | Semafori           | Utente    | Kernel         | `P()` / `V()`             |
| Fine Giornata   | Segnali            | Master    | Figli (Gruppi) | `SIGUSR2` (Evasione Code) |

## 4. Error Handling & Resilienza

- **EINTR:** Tutte le chiamate bloccanti (`msgrcv`, `semop`) devono essere racchiuse in un loop `while (errno == EINTR)`.
- **Termination:** In caso di `SIGINT` o `SIGTERM`, il Master forza l'apertura di tutti i cancelli (`GATE = 0`) prima di procedere al cleanup, sbloccando i figli che altrimenti rimarrebbero "zombie" in attesa semaforica.
