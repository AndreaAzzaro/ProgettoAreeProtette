# Registro di Implementazione Logica Atomica

Questo documento elenca ogni singolo passo incrementale della simulazione.
Ogni blocco di codice aggiunto DEVE rispettare i commenti qui definiti.

## ATOMO 0: Startup & Handshake (VERIFICATO)

**Stato:** Completato.
**Logica:** Il Master aspetta tutti i figli; i figli segnalano la presenza e superano il gate iniziale.
**Commenti nel Codice:**

- `/* 1. Sincronizzazione di Startup Globale */`
- `/* 2. Verifica superamento tornello iniziale */`

---

## ATOMO 1: Apertura Cancello Mattutino (Master)

//inizializza le barriere mattutine e serali
//inizializza la barriera pre lavoro
//lancia i processi
//tutti sbattono nella barriera pre lavoro
//! qui il direttore una volta svegliato dal suo semaforo ha tempo per fare tutto quello che vuole
//il direttore sposta da 1 a 0 il mutex
// superata la barriera pre lavoro inizierá il ciclo delle giornata lavorative

while(giornate lavorative){
//tutti i processi dovranno sbattere qui nella barriera mattuttina
//il direttore si sveglia una volta che tutti i figli hanno sbattuto
//ha tempo di fare qualsiasi cosa
//deve impostare il timer di posix per il signal per il ciclo di otto ore lavorative
//resetta il semaforo della sera a n_users
//sposta il semaforo da 1 a 0
while(otto ore lavorative){
setta il signal timer per il refill
si mette in pausa
arriva un segnale(timer refill, ctrl+c, threashold utenti, timer otto ore, add users)
handle del segnale
}
//manda il signal ai figli che la giornata sia finita(SIGUSR2)
//i processi sbattono sulla barriera
//il direttore si sveglia
//controlla la flag di add user, si blocca in un semaforo finché gli add_users nons crivono tutti i messaggi, poi fa getValMessage per ottenere il valore di quanti messaggi deve leggere e si mette in un ciclo di lettura
//legge la coda e vede quanti processi utenti si devono aggiungere e aggiorna il valore di n_users che si trova in shm
//resetta la barriera mattutina a n_users
//in un altrop ciclo manda nella coda il permesso ad add_users di creare i processi nuovi, tramite semaforo(che andranno a sbattere contro la barriera del mattino anche se il direttore sta modificando o lavorando con quella della sera a riga 48)
//ha tempo per fare qualsiasi cosa
//sposta da 1 a 0 la barriera della sera
}

-//! nel caso avvenga un threashold o un ctrl+c se il direttore si trovasse nel ciclo interno, deve fare l'handle del signal, quindi la riga 37 non é solo fare il refill, ma un handle di qualsiasi signal e in base al codice o fa il refill e continua il ciclo oppure le otto ore di lavoro diventano false, giornate lavorative diventano false ed esce dal ciclo, fa il signal di fine giornata per svegliare tutti i processi, si prosegue con la stessa logica da riga 39 a 44, ma poi il ciclo finisce perché giornate lavorative é diventato falso|
+//! GESTIONE EMERGENZA (CTRL+C / SIGINT):
+//! 1. L'handler dei segnali deve settare giornate_lavorative = 0 e otto_ore_lavorative = 0 (Punto 2 & 4).
+//! 2. Il Direttore deve forzare l'apertura di TUTTI i cancelli (MORNING_GATE=0, EVENING_GATE=0) per liberare i processi bloccati (Punto 2).
+//! 3. La coda di controllo deve essere rimossa per sbloccare eventuali add_users in attesa (Punto 3).
+//! 4. Se giornate_lavorative è 0, il Direttore salta i ricalcoli di popolazione e i reset delle barriere del mattino, procedendo dritto allo sgombero finale (Punto 4).

- //! add_users userá una logica simile a ctrl+c e threashold, peró renderá falso solo le otto ore lavorative e cambierá il valore della flag add_user
  //!creare una nuova coda di messaggi

Codice da aggiungere e modificare:

### 1. In `include/common.h`

```c
/**
 * @brief Payload per la coda di controllo (add_users -> Direttore).
 */
typedef struct {
    int users_count;           /**< Numero di utenti da aggiungere */
} ControlPayload;

/**
 * @brief Aggiunte alla struttura MainSharedMemory per la gestione dinamica.
 */
struct MainSharedMemory {
    // ...
    int control_queue_id;      /**< ID Coda per richieste add_users */
    int current_total_users;   /**< Numero attuale di utenti nella simulazione */
    // ...
};

/**
 * @brief Nuova risorsa semaforica per il permesso.
 */
typedef enum {
    // ... altri mutex ...
    MUTEX_ADD_USERS_PERMISSION, /**< Inizializzato a 0. V() dal Direttore, P() da add_users */
    MUTEX_SEMAPHORE_COUNT
} MutexSemaphoreIndex;
```

### 2. In `src/programs/responsabile_mensa/setup_ipc.c`

```c
// Inizializzare shm_ptr->current_total_users = config.number_of_initial_users
// Inizializzare shm_ptr->control_queue_id = create_message_queue(...)
// Inizializzare MUTEX_ADD_USERS_PERMISSION a 0
```

responsabile:
// [!] Flags: is_simulation_running e daily_cycle_is_active devono essere volatile sig_atomic_t
while (shm_ptr->is_simulation_running && shm_ptr->current_simulation_day < shm_ptr->configuration.timings.simulation_duration_days) {
setup_sigchld_handler() // [PUNTO 4] Gestisce morti asincrone dei figli
wait_for_zero(semaforo a n processi)
arm_daily_timer()
setup_barrier(semaforo della sera)
setup_signal_close_day()// qui fa il setup dei segnali di threashold, ctrl+c
setup_refill_signal()//qui imposta un timer casuale per il refill che puo andare dai 5 ai 30 minuti
open_barrier_gate(semaforo del mattino)

while (daily_cycle_is_active && shm_ptr->is_simulation_running) {
pause() // si addormenta e aspetta i signal (PUNTO 2: atomico tramite sig_atomic_t)
// Se riceve SIGCHLD: aggiorna n_users in SHM e ricalcola barriere correnti
}

broadcast_signal_to_all_groups(int signal)
wait_for_zero(semaforo a n processi)

if(add_user) { // gestisce add users
// [PUNTO 3] Legge dalla coda e V(semaforo posti_liberi_coda) per ogni messaggio letto
// Svuota la coda completamente prima di procedere
}

setup_barrier(semaforo del mattino) // [PUNTO 1] Reset Morning PRIMO di aprire la Evening

if(add_users) {
// Manda permesso a sbloccare processi add_users bloccati in msgrcv o semop
}
open_barrier_gate(semaforo della sera)
}

void broadcast_signal_to_all_groups(int signal) {
for (int i = 0; i < MAX_PROCESS_GROUPS; i++) {
pid_t pgid = shm_ptr->process_group_pids[i];
if (pgid > 0) {
kill(-pgid, signal); // Invia il segnale a tutto il gruppo
}
}
printf("[DEBUG] Notifica inviata a tutti i gruppi (Segnale: %d)\n", signal);
}

figli:
// [!] Flag: local_daily_flag deve essere volatile sig_atomic_t [PUNTO 2]
while(shm_ptr->is_simulation_running){
reserve_sem(semaforo a n processi)
local_daily_flag = true;
configure_child_signal()//caso SIGUSR2 fine giornata, SIGINIT/SIGTERM entrambe le flag a 0
wait_for_zero(semaforo a 1)

while(local_daily_flag){
if(local_daily_flag) {
// [PUNTO 3] Se add_users: P(semaforo posti_liberi_coda) prima di msgsnd
pause() // simulazione logica lavoro
}
}

reserve_sem(semaforo a n processi)
wait_to_zero(semaforo a 1)
}

//! NOTE AGGIUNTIVE STRESS TEST:
//! 1. PUNTO 3: Usiamo un semaforo inizializzato a MAX_QUEUE_SIZE. add_users fa P(), Direttore fa V().
//! 2. PUNTO 4: In caso di SIGCHLD, il Direttore aggiorna n_users in SHM. Se il Direttore è bloccato in wait_for_zero su una barriera,
//! l'handler del segnale deve forzare l'uscita o compensare il decremento mancante per evitare deadlock.
