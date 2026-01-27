# Analisi Funzionalità: Componente `add_users`

Questo documento raccoglie l'analisi tecnica, i requisiti e il protocollo di comunicazione per l'implementazione del modulo `add_users`, secondo quanto definito dal `logic_blueprint.md` e dai requisiti della versione "completa" del progetto.

## 1. Obiettivo

Permettere l'aggiunta dinamica di un numero $N$ di nuovi utenti (`N_NEW_USERS`) alla simulazione in corso senza dover riavviare il sistema.

## 2. Componenti Coinvolti

- **Eseguibile `add_users`**: Utility esterna invocata dall'utente.
- **Processo Master**: Gestore della simulazione (Engine).
- **Control Queue**: Coda IPC di messaggi dedicata alle richieste di controllo.
- **Shared Memory (SHM)**: Flag di stato e contatori globali.
- **Semaforo di Permesso**: Coordinamento temporale tra Master e Utility.

## 3. Protocollo di Comunicazione (Handshake)

### Fase A: Richiesta (Utility `add_users`)

1. L'utility si collega alla SHM tramite l'ID passato come argomento.
2. Invia un messaggio di tipo `ControlPayload` alla coda `control_queue_id` contenente il numero di utenti da aggiungere.
3. Imposta la flag atomica `shm->add_users_flag = 1`.
4. Invia un segnale `SIGUSR1` al Master per informarlo che c'è una richiesta pendente.
5. Si blocca in un'operazione `P()` (reserve) sul semaforo `MUTEX_ADD_USERS_PERMISSION`.

### Fase B: Gestione (Master - Engine)

1. Il Master riceve `SIGUSR1`. L'handler imposta `daily_cycle_is_active = 0`, forzando la fine prematura della giornata lavorativa corrente (simile all'overload).
2. Il Master esegue la chiusura standard della giornata:
   - Invia `SIGUSR2` (Fine Giorno) a tutti i figli esistenti.
   - Attende la sincronizzazione sulla barriera serale (`BARRIER_EVENING_READY`).
3. **Gestione Nuovi Arrivi**:
   - Rileva `shm->add_users_flag == 1`.
   - Legge tutti i messaggi dalla `control_queue_id`.
   - Per ogni richiesta, incrementa `shm->current_total_users`.
4. **Preparazione Nuovo Giorno**:
   - Esegue il reset della barriera mattutina (`setup_barrier`) usando il nuovo valore aggiornato di `shm->current_total_users`.
   - Esegue una `V()` (release) sul semaforo `MUTEX_ADD_USERS_PERMISSION` per dare il via libera all'utility.
   - Reimposta `shm->add_users_flag = 0`.

### Fase C: Creazione (Utility `add_users`)

1. L'utility si sblocca dal semaforo.
2. Esegue il fork/exec dei nuovi $N$ processi `utente`.
3. I nuovi processi `utente` si inizializzano e tentano di sincronizzarsi sulla barriera mattutina (che il Master ha già preparato per loro).
4. L'utility termina il suo compito.

## 4. Strutture Dati (IPC)

### Messaggio di Controllo

```c
typedef struct {
    long mtype;                /* Sempre 1 per richieste standard */
    int users_count;           /* Numero di utenti da aggiungere */
} ControlMessage;
```

### Shared Memory

```c
struct MainSharedMemory {
    // ...
    int control_queue_id;      /* ID della coda di controllo */
    int current_total_users;   /* Somma utenti iniziali + aggiunti */
    int add_users_flag;        /* Flag atomica (0/1) */
    // ...
};
```

## 5. Casi Critici e Soluzioni

- **Saturazione Registry**: Il Master deve verificare che `shm->current_total_users + N <= MAX_USERS_REGISTRY`.
- **Sincronizzazione Barriere**: Se i nuovi utenti non venissero conteggiati nel reset della barriera mattutina, il Master aspetterebbe all'infinito (Deadlock). Questo è risolto aggiornando `shm->current_total_users` _prima_ della `setup_barrier`.
- **Morte del Master**: Se il Master muore mentre l'utility aspetta sul semaforo, l'utility deve gestire il timeout o essere sbloccata dal cleanup delle risorse.

## 6. Prossimi Step Implementativi

1. Implementazione logica di ricezione segnale `SIGUSR1` nel Master.
2. Implementazione della logica di svuotamento coda nel `simulation_engine.c`.
3. Scrittura del programma `add_users.c`.
4. Test di aggiunta durante una simulazione attiva.
