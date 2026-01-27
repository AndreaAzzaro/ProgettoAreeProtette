# SYSTEM INSTRUCTIONS & BEHAVIORAL CONSTITUTION

Questo documento definisce le **REGOLE D'ORO (GOLDEN RULES)** per lo sviluppo di questo progetto.
L'Agente è tenuto a consultare queste direttive PRIMA di generare o modificare qualsiasi codice.

## 🚨 1. PROTOCOLLI DI SICUREZZA E ARCHITETTURA (Non-Negotiable)

- **NO POLLING:** È severamente vietato l'uso di attese attive. I processi devono dormire sui semafori.
- **NO ZOMBIES:** Ogni `fork()` deve avere una logica di gestione `wait` o `SIGCHLD` associata.
- **IPC SAFETY:**
  - È vietato usare `semget`, `semop`, `semctl` nudi nel codice di business.
  - L'agente DEVE usare esclusivamente i wrapper definiti in `include/sem.h`.
  - Le operazioni su Shared Memory devono essere atomiche e protette dai semafori definiti.
- **DOUBLE PROTOTYPE PATTERN:**
  - Ogni funzione pubblica deve essere definita nel suo modulo `.h` (es. `setup_ipc.h`).
  - Deve essere ANCHE richiamata/commentata nella facade `responsabile_mensa.h` per centralizzare la vista del Master.

## 🎨 2. STILE DI CODIFICA E DOCUMENTAZIONE

- **DOXYGEN OBBLIGATORIO:** Ogni funzione negli header (`.h`) deve avere commenti `@brief`, `@param`, `@return`.
- **LAYOUT DEL CODICE (Strict Order):**
  1.  Macro e Costanti (`#define`)
  2.  Variabili Globali e `extern`
  3.  Gestori di Segnali (Signal Handlers)
  4.  Funzioni Private (`static`)
  5.  Funzioni Pubbliche
  6.  **MAIN** (Se presente, va subito dopo le globali/handler, prima delle altre funzioni implementate).
- **LIMITI DI FUNZIONE:**
  - Massimo **50 righe** di codice effettivo per funzione.
  - Se eccede -> Refactoring obbligatorio in sotto-funzioni helper.
- **FLOW CONTROL:**
  - `continue`: **VIETATO**.
  - `break`: Consentito **SOLO** all'interno di `switch-case`.
  - Tutto il resto deve usare logica condizionale (`if/else`) pulita.
- **NAMING:** Variabili e funzioni devono essere esplicite (es. `shared_memory_id`, non `shmid`). Niente abbreviazioni criptiche.

## ⚙️ 3. WORKFLOW OPERATIVO DELL'AGENTE

- **PROPOSAL FIRST:** Non modificare MAI fisicamente i file senza aver prima proposto la modifica o lo scheletro in chat e aver ricevuto un "procedi".
- **ERROR FIRST:** Se un comando (es. compilazione) fallisce, interrompi subito il processo. Spiega l'errore tecnico e proponi la fix. Non tentare fix "alla cieca".
- **LOGIC PRESERVATION:** Non rimuovere mai commenti o logica esistente per "pulizia" senza permesso esplicito.
- **CROSS-CHECK:** Ogni decisione deve rispettare la logica definita in `.agent/logic_blueprint.md`.

## 📝 4. GESTIONE ORDINI (Rule specific)

- **ID EXCHANGE:** Nelle code di messaggi si scambiano SOLO ID numerici, mai stringhe.
- **MAPPING:** Usa la `SimulationMenu` in Shared Memory per convertire ID -> Testo.
