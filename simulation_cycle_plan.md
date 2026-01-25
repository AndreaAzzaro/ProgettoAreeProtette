# Piano di Azione: Ciclo di Simulazione Basato su Segnali

L'obiettivo di questo documento è definire la logica di orchestrazione tra il **Master** (Responsabile Mensa) e i **Figli** (Utenti, Operatori, Cassieri) per gestire i giorni di simulazione in modo persistente e reattivo.

## 1. Architettura della Barriera "Ping-Pong" + Segnali

Il ciclo giornaliero si divide in tre fasi critiche coordinate dai semafori del set `semaphore_sync_id`.

### Fase A: La Mattina (Sincronizzazione di Ingresso)

1. **Figli:** Chiamano `sync_child_start(MORNING_READY, MORNING_GATE)`. Si bloccano finché la mensa non "apre".
2. **Master:**
   - Sblocca il `MORNING_GATE` (N.B. Il setup di questa barriera è stato fatto la sera precedente o al bootstrap).
   - **Immediatamente dopo**, prepara la barriera della **Sera** (`BARRIER_EVENING_READY/GATE`).

### Fase B: Il Giorno (Esecuzione e Timer)

1. **Figli:** Eseguono il loro loop interno `while (daily_cycle_is_active)`.
2. **Master:**
   - Esegue un loop di minuti: `for (m=0; m < OPENING_TIME; m++)`.
   - Ad ogni iterazione attende `NNANOSECS`.
   - Gestisce eventi asincroni (es. Refill ogni 10 minuti).

### Fase C: La Sera (Sgombero via Segnali e Barriera)

1. **Master (Allo scadere del timer):**
   - Invia `SIGUSR2` a tutti i gruppi di processi: `kill(-pgid, SIGUSR2)`.
   - **Nota Bene:** Questo non uccide i processi, ma serve a svegliarli dalle chiamate bloccanti (es. attesa in coda) e a farli uscire dall'orario di lavoro giornaliero.
   - Il Master sblocca la barriera `EVENING_GATE` (già preparata al mattino).
   - **Immediatamente dopo**, prepara la barriera della **Mattina** successiva (`BARRIER_MORNING_READY/GATE`).
2. **Figli (Ricevuto SIGUSR2):**
   - La flag locale `daily_cycle_is_active` viene impostata a `0` dall'handler.
   - Il processo interrompe le attività correnti (se era in coda, abbandona).
   - Chiama `sync_child_start(EVENING_READY, EVENING_GATE)` per confermare lo sgombero al Master.
   - **Persistence:** Il processo torna alla sommità del loop principale e si rimette in attesa sulla `BARRIER_MORNING_GATE` per il giorno successivo.

---

## 2. Bozze Logica `simulation_engine.c` (Master)

```c
void run_simulation_loop(MainSharedMemory *shm) {
    /* Setup Iniziale della primissima barriera mattutina prima di entrare nel loop */
    setup_barrier(shm, BARRIER_MORNING_READY, BARRIER_MORNING_GATE, TOTAL_PROCESSES);

    while (shm->current_simulation_day < shm->configuration.sim_duration && shm->is_simulation_running) {

        /* --- 1. APERTURA MATTINA --- */
        handle_new_day(shm);
        sync_master_unlock_barrier(shm, BARRIER_MORNING_READY, BARRIER_MORNING_GATE);

        // Setup anticipato per la sera (Mentre i figli lavorano)
        setup_barrier(shm, BARRIER_EVENING_READY, BARRIER_EVENING_GATE, TOTAL_PROCESSES);

        printf("[SYSTEM] Mensa aperta per il Giorno %d\n", shm->current_simulation_day);

        /* --- 2. GESTIONE TEMPO (EVENT-DRIVEN) --- */
        // Sostituiamo il loop con un'attesa guidata da segnali del kernel
        printf("[SYSTEM] Avvio Timer Giornaliero (%d min)\n", total_minutes);

        // A. Imposta timer a scadenza per la fine della giornata (Invia SIGALRM)
        setup_day_end_timer(shm->configuration.minutes_per_day);

        // B. Imposta timer periodico per il refill (Ogni 10 min)
        setup_refill_timer(10);

        /* Loop di attesa passiva: Il Master dorme finché un segnale lo sveglia */
        while (day_in_progress) {
            sigsuspend(&normal_mask); // Attesa efficiente (CPU = 0)

            // Se siamo stati svegliati dal timer di Refill:
            if (event_check == EVENT_REFILL) {
                handle_refill_cycle(shm);
                event_check = 0; // Reset evento
            }

            // Se siamo stati svegliati dal timer Fine Giornata:
            if (event_check == EVENT_DAY_END) {
                day_in_progress = 0; // Esci dal loop interno
            }
        }

        /* --- 3. CHIUSURA SERALE --- */
        printf("[SYSTEM] Orario di chiusura raggiunto. Notifica ai figli...\n");
        // ... (Invio segnali ai figli coordinato)

        // Sblocco sera
        sync_master_unlock_barrier(shm, BARRIER_EVENING_READY, BARRIER_EVENING_GATE);

        /* --- 4. VERIFICA OVERLOAD (Consegna riga 246) --- */
        // Il controllo va fatto "al termine della giornata"
        if (get_users_in_queue_count(shm) > shm->configuration.overload_threshold) {
             printf("[ALERT] Overload rilevato a fine giornata! Terminazione...\n");
             shm->is_simulation_running = 0;
             break;
        }

        // Setup anticipato per la mattina successiva
        setup_barrier(shm, BARRIER_MORNING_READY, BARRIER_MORNING_GATE, TOTAL_PROCESSES);

        /* --- 5. REPORT GIORNALIERO --- */
        collect_and_print_daily_stats(shm);
    }
}
```

## 3. Punti di Forza di questo Piano

- **Resilienza:** Se un processo è bloccato in una coda (`msgrcv`), il segnale `SIGUSR2` lo risveglia forzatamente.
- **Sincronia:** La barriera finale (`EVENING`) garantisce che il Master inizi a calcolare le statistiche solo quando l'ultimo utente è uscito dalla memoria condivisa.
- **Isolamento:** La logica temporale risiede solo nel Master, i figli reagiscono solo ai segnali.
