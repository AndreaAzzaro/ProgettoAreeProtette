/**
 * @file ipc_keys.h
 * @brief Definizioni degli ID fissi per le risorse IPC.
 *
 * Questo file contiene le chiavi statiche per semafori, code di messaggi e
 * memoria condivisa, facilitando il debugging e il monitoraggio tramite ipcs.
 *
 * Organizzazione degli ID:
 * - 1000-1999: Semafori
 * - 2000-2999: Code di messaggi
 * - 3000-3999: Memoria condivisa
 *
 * NOTA: Con ID fissi, è necessario effettuare cleanup manuale delle risorse
 * IPC tra le esecuzioni usando gli script forniti o il comando ipcrm.
 */

#ifndef IPC_KEYS_H
#define IPC_KEYS_H

/* ==========================================================================
 *                    SEMAFORI (Range 1000-1999)
 * ========================================================================== */

/** ID del set di semafori per le barriere di sincronizzazione giornaliera */
#define IPC_KEY_SEMAPHORE_SYNC              1000

/** ID del set di semafori mutex globali */
#define IPC_KEY_SEMAPHORE_MUTEX             1100

/** ID del pool di semafori per la sincronizzazione dei gruppi di utenti */
#define IPC_KEY_SEMAPHORE_GROUP_POOL        1200

/** ID del semaforo per i validatori ticket all'ingresso */
#define IPC_KEY_SEMAPHORE_TICKET            1300

/** ID del semaforo di condizione per la disponibilità posti a sedere */
#define IPC_KEY_SEMAPHORE_DINING_AREA       1400

/** ID del set di semafori della stazione primi piatti */
#define IPC_KEY_SEMAPHORE_FIRST_STATION     1500

/** ID del set di semafori della stazione secondi piatti */
#define IPC_KEY_SEMAPHORE_SECOND_STATION    1600

/** ID del set di semafori della stazione caffè/dolci */
#define IPC_KEY_SEMAPHORE_COFFEE_STATION    1700

/** ID del set di semafori della stazione cassa */
#define IPC_KEY_SEMAPHORE_REGISTER_STATION  1800

/* ==========================================================================
 *                    CODE DI MESSAGGI (Range 2000-2999)
 * ========================================================================== */

/** ID della coda di messaggi della stazione primi piatti */
#define IPC_KEY_QUEUE_FIRST_STATION         2000

/** ID della coda di messaggi della stazione secondi piatti */
#define IPC_KEY_QUEUE_SECOND_STATION        2100

/** ID della coda di messaggi della stazione caffè/dolci */
#define IPC_KEY_QUEUE_COFFEE_STATION        2200

/** ID della coda di messaggi della stazione cassa */
#define IPC_KEY_QUEUE_REGISTER_STATION      2300

/** ID della coda di controllo per le richieste add_users */
#define IPC_KEY_QUEUE_CONTROL               2400

/* ==========================================================================
 *                    MEMORIA CONDIVISA (Range 3000-3999)
 * ========================================================================== */

/** ID della memoria condivisa principale della simulazione */
#define IPC_KEY_SHARED_MEMORY               3000

#endif /* IPC_KEYS_H */
