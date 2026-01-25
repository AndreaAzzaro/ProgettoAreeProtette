# Logic Blueprint - Progetto Aree Protette / Mensa

Questo documento funge da "Memoria Centrale" per Antigravity. Deve essere consultato prima di ogni modifica strutturale per garantire la coerenza della logica di programmazione, la gestione delle IPC e la gerarchia dei processi.

## 1. Architettura dei Processi

- **Responsabile Mensa (Master):** Coordinatore principale, gestisce IPC e spawn dei processi figli. Utilizza un'architettura a **Moduli (Facade)** centralizzata in `responsabile_mensa.h`.

### Architettura del Responsabile Mensa

Per gestire la complessità del processo Master, il codice è diviso in sotto-moduli operativi:

- `setup_ipc`: Creazione e inizializzazione di SHM, Semafori e Code.
- `setup_population`: Calcolo distribuzione e spawn di Operatori e Utenti.
- `simulation_engine`: Ciclo di vita della simulazione (Giorno/Notte, Refill).

**Double Prototype Pattern:**
Ogni funzione dei sotto-moduli deve seguire questo schema:

1.  **Definizione Dettagliata:** Nel file `.h` del modulo (es. `setup_ipc.h`) con JSDoc/Doxygen completo.
2.  **Puntamento (Facade):** In `responsabile_mensa.h`, si ripete il prototipo con un commento che indica l'origine (es. `/* Definizione in setup_ipc.h */`). Questo permette di avere una vista globale di tutte le capacità del Master in un unico file.

- **Operatore:** Processi dedicati al servizio e alla gestione delle scorte.
- **Utente:** Processi che usufruiscono della mensa.
- **Operatore Cassa:** Gestione pagamenti/flusso finale.

## 2. Gestione IPC (Semafori e SHM)

_(Sezione da compilare seguendo le istruzioni dell'utente)_

## 3. Logica di Sincronizzazione

### Startup: Pattern Double Barrier Gate

Utilizzato per garantire che nessun processo figlio inizi l'esecuzione della simulazione vera e propria finché tutti i figli non sono stati spawnati e inizializzati.

**Attori:**

1.  **Responsabile Mensa (Padre):**
    - Inizializza un semaforo (es. `INIT_SEM_READY`) a $N$ (numero totale di processi figli attesi).
    - Inizializza un semaforo 'cancello' (es. `INIT_SEM_START`) a 1.
    - Esegue `wait_for_zero(INIT_SEM_READY)`.
    - Una volta svegliato (tutti i figli pronti), imposta `INIT_SEM_START` a 0.
2.  **Processi Figli (Operatori, Utenti, ecc.):**
    - Eseguono `sync_child_start(sem_id, READY_IDX, START_IDX)`.
    - Internamente: decrementano `READY_IDX` e si bloccano in `wait_for_zero(START_IDX)`.

**Vantaggio:** Previene race conditions durante l'accesso iniziale alle IPC e assicura una partenza "in linea" di tutta la popolazione della simulazione.

**⚠️ Caveat & Sicurezza:**

- **Rischio Deadlock:** Se un processo figlio muore _prima_ di chiamare `sync_child_start`, il semaforo `READY` non arriverà mai a zero, bloccando il Padre all'infinito.
- **Strategia di Mitigazione:** Implementare in `simulation_engine.c` un handler per `SIGCHLD` o un meccanismo di timeout che permetta al Padre di interrompere l'attesa e ripulire le risorse in caso di errore fatale durante lo spawn.

### Ciclo Giornaliero: Ping-Pong Barrier (Reset Incrociato)

Per gestire più giorni di simulazione, si utilizzano due set di barriere (Mattina e Sera) che si auto-resettano a vicenda.

**Inizializzazione (Post-Boot):**

- Il Direttore inizializza `MORNING_READY` a $N$, `MORNING_GATE` a 1.
- Il Direttore inizializza `EVENING_READY` a $N$, `EVENING_GATE` a 1.

**Fase Mattina (Inizio Giorno):**

1.  **Direttore**: Resetta il semaforo `EVENING_READY` riportandolo a $N$ e `EVENING_GATE` a 1.
2.  **Direttore**: Esegue `wait_for_zero(MORNING_READY)`.
3.  **Figli**: Eseguono `sync_child_start` sulla barriera `MORNING`.
4.  **Direttore**: Apre il cancello mattutino (`MORNING_GATE = 0`).

**Fase Sera (Fine Giorno):**

1.  **Direttore**: Resetta il semaforo `MORNING_READY` a $N$ e `MORNING_GATE` a 1.
2.  **Direttore**: Esegue `wait_for_zero(EVENING_READY)`.
3.  **Figli**: Eseguono `sync_child_start` sulla barriera `EVENING`.
4.  **Direttore**: Apre il cancello serale (`EVENING_GATE = 0`).

**Vantaggio Tecnico:** Il "Reset Incrociato" garantisce che la barriera inattiva sia sempre pronta e pulita prima che i processi la raggiungano, eliminando ogni possibile asincronia tra i giorni.

## 4. Gestione Segnali

### Pattern di Controllo Asincrono

La simulazione utilizza i segnali UNIX per gestire eventi imprevisti o interruzioni globali senza ricorrere al polling.

**Utilizzo nel Responsabile Mensa (Direttore):**

- **Inizializzazione:** Chiama `configure_director_signal_handlers()` all'avvio del `Main`.
- **Terminazione:** Monitora `termination_requested_flag`. Se impostata a 1 (via `SIGINT` o `SIGTERM`), avvia la procedura `terminate_simulation_gracefully()`.
- **Rifornimento:** Monitora `refill_request_flag` (innescata da `SIGUSR2`) per coordinare il rifornimento delle stazioni se richiesto esternamente.

**Utilizzo nei Processi Figli (Operatori, Utenti):**

- **Inizializzazione:** Chiamano `configure_child_signal_handlers()` passando i riferimenti alle proprie flag di loop (`simulation_running_status`, `daily_cycle_running_status`).
- **Reattività:** Gli handler aggiornano i puntatori forniti, permettendo l'uscita immediata dai cicli `while` in caso di segnali di terminazione globale, garantendo una chiusura pulita e coordinata di tutta la gerarchia dei processi.

## 5. Regole d'Oro (Non violabili)

1. **Strict Command Adherence:** Non eseguire analisi proattive o modifiche a file non citati esplicitamente nel prompt corrente.
2. **Doxygen Documentation:** Ogni header (`.h`) creato o pulito deve obbligatoriamente includere commenti in stile Doxygen (`@brief`, `@param`, `@return`) per ogni funzione e struttura esportata.
3. **IPC Modularity:** È severamente vietato riscrivere logica di manipolazione semaforica (`semget`, `semop`, `semctl`) all'interno dei processi. La logica dei processi deve affidarsi esclusivamente alle funzioni definite in `include/sem.h` e implementate in `src/ipc/sem.c`.
4. **Nessun File Zombie:** Ogni processo deve gestire la propria terminazione e pulizia.
5. **Atomicità:** Le operazioni su memoria condivisa devono essere sempre protette dai semafori definiti.
6. **Stazioni Distribuzione:** Ogni stazione ha la sua coda di messaggi (`message_queue_id`), il suo set di semafori (`semaphore_set_id`) e un array di porzioni disponibili.

### Gestione Ordini e Risoluzione Piatti

Per rendere efficiente la comunicazione tra processi tramite le code di messaggi:

- **ID Exchange:** I processi (es. Utenti e Operatori) non si scambieranno stringhe di testo (troppo pesanti e rischiose), ma esclusivamente **ID numerici** dei piatti.
- **Mapping Table Persistence:** Grazie alla struttura centralizzata in `SimulationMenu` (caricata in Shared Memory), ogni processo potrà usare l'ID ricevuto per:
  1. Identificare istantaneamente la categoria del piatto.
  2. Ricavare il nome testuale del piatto per il logging o la visualizzazione, accedendo all'array corrispondente in memoria condivisa.
- **Coerenza Interna:** Il sistema di mapping definito in `src/config/menu.c` deve essere lo standard unico per generare e interpretare questi ID in tutta la simulazione.

7. **Step-by-Step Reporting:** Ogni aggiunta a questo documento deve essere prima confermata o richiesta dall'utente.
8. **Clean Flow Control:** È severamente vietato l'uso di polling (attese attive) e `continue`. L'uso di `break` è consentito **esclusivamente** all'interno dei costrutti `switch-case`. In ogni altro contesto, il flusso deve essere gestito tramite logica condizionale pulita.
9. **Explicit Naming Convention:** È obbligatorio usare nomi di variabili, funzioni e parametri chiari ed espliciti. Sono vietate le abbreviazioni.
10. **Logic Preservation:** È severamente vietato modificare la logica strutturale esistente senza permesso.
11. **Proposal First Policy:** Prima di apportare qualsiasi modifica fisica ai file del codice, è obbligatorio mostrare un esempio o una bozza della modifica in chat. L'applicazione effettiva sui file può avvenire solo dopo il comando esplicito dell'utente (es. "procedi").
12. **No Redundancy Removal:** È vietato eliminare righe di codice o logica esistente con il pretesto della "pulizia" senza avvisare esplicitamente l'utente. Ogni modifica ai nomi deve mantenere l'integrità dei dati esistenti; nel dubbio, è preferibile aggiungere piuttosto che togliere.
13. **Code Layout Order:** In ogni file, l'ordine degli elementi deve essere rigorosamente:
    1. Macro e Costanti (`#define`).
    2. Variabili Globali (se necessarie) e `extern`.
    3. Gestori di Segnali (Signal Handlers).
    4. Funzioni Private (`static`).
    5. Funzioni Pubbliche.
       Questo garantisce che la configurazione e lo stato globale siano visibili prima della logica.
14. **Error First Policy (Compilation):** In caso di errore di compilazione (`gcc` failure), è obbligatorio interrompere qualsiasi azione automatica. Bisogna spiegare la causa tecnica dell'errore all'utente e proporre una soluzione formale in chat. La correzione fisica del file può avvenire solo dopo l'esplicito "procedi" dell'utente.
15. **Main First Strategy:** In ogni file sorgente `.c` contenente un punto di ingresso, la funzione `main` deve essere posizionata all'inizio (subito dopo gli include e le variabili globali/statiche). Tutte le altre funzioni locali devono essere implementate sotto il `main`.
16. **Function Shortness Policy:** Ogni funzione deve avere una lunghezza massima di 50 righe (escluse intestazioni JSDoc/Doxygen). Se la logica eccede questo limite, è obbligatorio scomporla in sotto-funzioni specializzate.
17. **Cross-Document Compliance:** Ogni decisione tecnica e architetturale deve derivare dalla sintesi di tre documenti: `Governance Rules` (Global), `.agent/logic_blueprint.md` e `consegna.md`. La violazione di uno solo di questi documenti è considerata un errore critico.
18. **Blueprint Persistence:** La logica definita nel `logic_blueprint.md` è immutabile e obbligatoria anche in "modalità emergenza", durante la ricostruzione di file persi o in caso di errori di sistema. Non sono ammesse semplificazioni o deviazioni senza esplicita approvazione.

## 6. Sviluppi Futuri (To-Do)

- **Mapping Avanzato:** Valutare la sostituzione del mapping `switch-case` in `config.c` con una tabella di puntatori ad offset di memoria per un caricamento ancora più scalabile e pulito.
