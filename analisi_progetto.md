# Analisi Progetto: Mensa "Oasi del Golfo"

## 1. Il Responsabile Mensa (Master)

### FASE 1: Setup e Inizializzazione Logica
In questa fase il Responsabile prepara l'ambiente prima di attivare i motori.

- **Configurazione**: Caricamento della struct `Config` da file (es. `config.conf`) per parametri come `SIMDURATION`, `NOFUSERS`, `NOFWORKERS`, `NNANOSECS`, ecc.
- **Risorse IPC**: Creazione fisica degli identificatori (`shmget`, `semget`, `msgget`).
- **Shared Memory (SHM)**: Inizializzazione della memoria globale che conterrà:
    - Array delle **Stazioni** (Stato porzioni, code, operatori attivi).
    - Risorse per i **Tavoli** (Conteggio posti liberi).
    - Variabili per le **Statistiche** giornaliere/totali.
- **Generazione Processi**:
    - **Operatori**: Assegnazione mansione fissa (Primi, Secondi, Caffè) passata come argomento in `argv`.
    - **Utenti**: 
        - Assegnazione Ticket Sconto (Target: 80% della popolazione totale) via `argv`.
        - Creazione logica dei **Gruppi**: Assegnazione di un `groupID` random agli utenti in fase di creazione (da 1 a `MAXUSERSPERGROUP`). Gli utenti con lo stesso ID cercheranno di sincronizzarsi per mangiare al tavolo.

### FASE 2: Barriera di Sincronizzazione (Start Contemporaneo)
Per garantire che nessun processo inizi a "lavorare" prima che l'ultimo sia stato creato.

- **Tecnica: Semaforo a valore 0 (Zero-Wait)**
    - Il Responsabile crea un set di semafori di sincronizzazione:
        1. `SEM_READY`: Inizializzato al numero totale di figli.
        2. `SEM_START`: Inizializzato a 0.
- **Protocollo**:
    1. Ogni **Figlio** (dopo l'inizializzazione interna) fa `semop(-1)` su `SEM_READY` e resta in `semop(-1)` (attesa) su `SEM_START`.
    2. Il **Responsabile** attende che `SEM_READY` arrivi a 0 (istruzione `semop` con valore 0).
    3. Quando tutti sono pronti, il Responsabile "apre i cancelli" con una singola `semop(+NO_FIGLI)` su `SEM_START`.
    4. Tutti i figli partono all'unisono.

---