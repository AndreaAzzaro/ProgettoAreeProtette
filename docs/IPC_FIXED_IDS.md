# Migrazione a ID IPC Fissi

## Motivazione

Il progetto è stato migrato da chiavi IPC generate con `ftok()` a **ID fissi predefiniti**. Questo cambiamento semplifica enormemente il debugging e il monitoraggio delle risorse IPC durante lo sviluppo.

### Vantaggi degli ID Fissi

✅ **Debug più facile**: Quando vedi un processo bloccato sul semaforo 1100 con `ipcs`, sai immediatamente che si tratta dei mutex globali
✅ **Prevedibilità**: Gli ID sono sempre gli stessi ad ogni esecuzione
✅ **Documentazione chiara**: Ogni ID corrisponde a una risorsa specifica e ben documentata
✅ **Tracciamento immediato**: Basta guardare l'ID per capire su quale risorsa un processo è in attesa

### Svantaggi (mitigati)

⚠️ **Cleanup manuale**: È necessario pulire le risorse IPC tra le esecuzioni (forniti script automatici)
⚠️ **Possibili conflitti**: Se altri programmi usano gli stessi ID (improbabile con i range scelti)

## Schema degli ID

### Semafori (1000-1999)

| ID   | Nome | Descrizione |
|------|------|-------------|
| 1000 | `semaphore_sync_id` | Barriere di sincronizzazione (startup/morning/evening) |
| 1100 | `semaphore_mutex_id` | Mutex globali (MUTEX_SIMULATION_STATS, MUTEX_SHARED_DATA, ecc.) |
| 1200 | `group_sync_semaphore_id` | Pool di semafori per sincronizzazione gruppi utenti |
| 1300 | `semaphore_ticket_id` | Validatori ticket all'ingresso (4 slot) |
| 1400 | `condition_semaphore_id` | Semaforo di condizione per posti a sedere |
| 1500 | `first_course_station.semaphore_set_id` | Set semafori stazione primi piatti |
| 1600 | `second_course_station.semaphore_set_id` | Set semafori stazione secondi piatti |
| 1700 | `coffee_dessert_station.semaphore_set_id` | Set semafori stazione caffè/dolci |
| 1800 | `register_station.semaphore_set_id` | Set semafori stazione cassa |

### Code di Messaggi (2000-2999)

| ID   | Nome | Descrizione |
|------|------|-------------|
| 2000 | `first_course_station.message_queue_id` | Coda ordini primi piatti |
| 2100 | `second_course_station.message_queue_id` | Coda ordini secondi piatti |
| 2200 | `coffee_dessert_station.message_queue_id` | Coda ordini caffè/dolci |
| 2300 | `register_station.message_queue_id` | Coda pagamenti cassa |
| 2400 | `control_queue_id` | Coda di controllo per add_users |

### Memoria Condivisa (3000-3999)

| ID   | Nome | Descrizione |
|------|------|-------------|
| 3000 | `shared_memory_id` | Memoria condivisa principale della simulazione |

## Script di Gestione IPC

### Visualizzare lo stato delle risorse

```bash
./scripts/ipc_status.sh
```

Mostra tutte le risorse IPC attive con i loro valori correnti. Output di esempio:

```
SEMAFORI
========================================
✓ Semaforo 1000 - Barriere di sincronizzazione (SYNC_BARRIER)
  └─ Set di 8 semafori
     [0] = 0
     [1] = 0
     ...

✓ Semaforo 1100 - Mutex globali (MUTEX)
  └─ Set di 4 semafori
     [0] = 1  (MUTEX_SIMULATION_STATS)
     [1] = 1  (MUTEX_SHARED_DATA)
     ...
```

### Pulire tutte le risorse IPC

```bash
./scripts/ipc_cleanup.sh
```

Rimuove tutte le risorse IPC del progetto. **Esegui questo script tra un'esecuzione e l'altra** per garantire un ambiente pulito.

## Debug con ipcs

### Trovare processi bloccati

```bash
# Mostra tutti i semafori con i loro ID
ipcs -s

# Dettagli su un semaforo specifico (es. mutex globali)
ipcs -s -i 1100

# Mostra tutte le code di messaggi
ipcs -q

# Dettagli su una coda (es. coda cassa)
ipcs -q -i 2300
```

### Interpretare rapidamente gli ID

Quando vedi un processo bloccato durante il debug:

- **1000**: Barriere → Processo in attesa di sincronizzazione giornaliera
- **1100**: Mutex → Sezione critica (controlla quale indice: 0=stats, 1=shared_data, 2=add_users, 3=tables)
- **1200**: Gruppi → Sincronizzazione pre-cassa/tavolo/uscita
- **1500-1800**: Stazioni → Operatore in attesa di postazione o utente in coda
- **2000-2300**: Code → Comunicazione ordini/pagamenti

## File Modificati

### Nuovi File
- [`include/ipc_keys.h`](../include/ipc_keys.h) - Definizioni degli ID fissi
- [`scripts/ipc_cleanup.sh`](../scripts/ipc_cleanup.sh) - Script di cleanup
- [`scripts/ipc_status.sh`](../scripts/ipc_status.sh) - Script di visualizzazione stato

### File Modificati
- [`src/programs/responsabile_mensa/setup_ipc.c`](../src/programs/responsabile_mensa/setup_ipc.c) - Usa ID fissi invece di ftok/IPC_PRIVATE
- [`src/programs/add_users/add_users.c`](../src/programs/add_users/add_users.c) - Rimosso ftok, usa IPC_KEY_SHARED_MEMORY
- [`src/programs/communication_disorder/communication_disorder.c`](../src/programs/communication_disorder/communication_disorder.c) - Rimosso ftok, usa IPC_KEY_SHARED_MEMORY

### File Deprecati
- `IPC_KEY_PATH` e `IPC_PROJECT_ID` in `common.h` - Mantenuti per retrocompatibilità ma non più utilizzati

## Note Importanti

1. **Cleanup obbligatorio**: Tra un'esecuzione e l'altra, esegui sempre `./scripts/ipc_cleanup.sh` oppure il programma principale potrebbe fallire con errori di risorse già esistenti.

2. **Compatibilità**: Il sistema di build è stato testato e compila senza errori o warning.

3. **Race condition debugging**: Con gli ID fissi, quando hai un processo che sembra bloccato, esegui:
   ```bash
   ipcs -s | grep " [0-9]\+ "
   ```
   E confronta gli ID con la tabella sopra per capire immediatamente su quale risorsa è in attesa.

4. **Estensibilità**: Per aggiungere nuove risorse IPC, aggiungi gli ID nel range appropriato in `include/ipc_keys.h` e aggiorna gli script di cleanup e status.
