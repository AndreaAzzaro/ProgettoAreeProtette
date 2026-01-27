# Logic Blueprint - Progetto Aree Protette / Mensa

> **⚠️ ATTENZIONE:** L'implementazione di questo progetto deve seguire rigorosamente le "Regole d'Oro" definite nel file `AGENTS.md`.

Questo documento funge da "Memoria Centrale" per la logica di dominio, la gestione delle IPC e la gerarchia dei processi.

## 1. Architettura dei Processi

- **Responsabile Mensa (Master):** Coordinatore principale. Architettura a **Moduli (Facade)** in `responsabile_mensa.h`.
  - `setup_ipc`: Creazione SHM, Semafori, Code.
  - `setup_population`: Spawn Operatori/Utenti.
  - `simulation_engine`: Ciclo vita (Giorno/Notte).

- **Operatore:** Processi dedicati al servizio e scorte.
- **Utente:** Processi che usufruiscono della mensa.
- **Operatore Cassa:** Gestione pagamenti.

## 2. Gestione IPC (Semafori e SHM)

_(Sezione dinamica da compilare durante lo sviluppo)_

## 3. Logica di Sincronizzazione

### Startup: Pattern Double Barrier Gate

Garantisce che la simulazione parta solo quando tutti i processi sono pronti.

1.  **Padre:** Attende su `INIT_SEM_READY` (che vale N). Quando arriva a 0, apre il cancello `INIT_SEM_START`.
2.  **Figli:** Decrementano `READY`, poi si bloccano su `START`.
3.  **Sicurezza:** Implementare timeout/SIGCHLD per evitare deadlock se un figlio muore all'avvio.

### Ciclo Giornaliero: Ping-Pong Barrier (Reset Incrociato)

Gestione alternata Mattina/Sera.

- **Mattina:** Si usa il set `MORNING`. Il Padre resetta preventivamente il set `EVENING`.
- **Sera:** Si usa il set `EVENING`. Il Padre resetta preventivamente il set `MORNING`.

## 4. Gestione Segnali (Controllo Asincrono)

- **Direttore:** Usa `termination_requested_flag` (SIGINT/SIGTERM) e `refill_request_flag` (SIGUSR2).
- **Figli:** Aggiornano puntatori alle flag di loop (`simulation_running_status`) per uscire pulitamente dai `while`.

## 5. Logica di Business (Mensa)

### Gestione Ordini e Menu

- **Struttura:** Centralizzata in `SimulationMenu` (Shared Memory).
- **Mapping:** Il sistema in `src/config/menu.c` è l'unico standard per generare ID.
- **Stazioni:** Ogni stazione ha:
  - 1 Coda messaggi (`message_queue_id`)
  - 1 Set semafori (`semaphore_set_id`)
  - Array porzioni disponibili.

## 6. Sviluppi Futuri (To-Do)

- [ ] Implementazione logica "Responsabile Mensa".
- [ ] Debug della "Double Barrier".
- [ ] Mapping avanzato (puntatori offset invece di switch-case).
