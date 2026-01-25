/**
 * @file common.h
 * @brief Definizioni globali, strutture dati della memoria condivisa e indici IPC.
 * 
 * Questo file centralizza tutte le strutture e le costanti necessarie per la 
 * comunicazione tra i diversi processi della simulazione (Responsabile, Operatori, Utenti).
 */

#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <unistd.h>
#include "config.h"
#include "statistics.h"
#include "menu.h"

/** Numero di postazioni per la validazione automatica dei ticket all'ingresso */
#define TICKET_VALIDATORS_COUNT 2

/**
 * @brief Indici per l'identificazione dei gruppi di processi.
 * Utile per la gestione dei segnali di gruppo (PGID).
 */
typedef enum {
    GROUP_CASHIERS,         /**< Gruppo degli Operatori di Cassa */
    GROUP_FIRST_COURSES,    /**< Gruppo degli Operatori dei Primi Piatti */
    GROUP_SECOND_COURSES,   /**< Gruppo degli Operatori dei Secondi Piatti */
    GROUP_DESSERT_COFFEE,   /**< Gruppo degli Operatori di Bar/Dolci */
    GROUP_USERS,            /**< Gruppo degli Utenti della mensa */
    MAX_PROCESS_GROUPS      /**< Numero totale di gruppi gestiti */
} ProcessGroupIndex;

/**
 * @brief Indici per il set di semafori della Barriera di Sincronizzazione Giornaliera.
 * Implementa il pattern "Ping-Pong Barrier" per l'avvio e la fine del giorno.
 */
typedef enum {
    BARRIER_STARTUP_READY = 0,  /**< Startup: Conteggio processi pronti all'avvio iniziale */
    BARRIER_STARTUP_GATE,       /**< Startup: Cancello di avvio globale (aperto dal Direttore) */
    BARRIER_MORNING_READY,      /**< Mattina: Conteggio processi pronti alla partenza giornaliera */
    BARRIER_MORNING_GATE,       /**< Mattina: Cancello di avvio giornaliero */
    BARRIER_EVENING_READY,      /**< Sera: Conteggio processi arrivati a fine giornata */
    BARRIER_EVENING_GATE,       /**< Sera: Cancello di chiusura/reset giorno e sblocco */
    SYNC_BARRIER_SEM_COUNT      /**< Totale semafori in questo set (incrementato a 6) */
} SyncBarrierIndex;

/**
 * @brief Indici per il set di semafori Mutex.
 * Garantiscono l'accesso atomico alle sezioni critiche della memoria condivisa.
 */
typedef enum {
    MUTEX_SIMULATION_STATS = 0, /**< Protegge la struttura delle statistiche globali */
    MUTEX_SHARED_DATA,          /**< Protegge i campi comuni della memoria condivisa (es. giorno corrente) */
    MUTEX_SEMAPHORE_COUNT       /**< Numero totale di semafori mutex */
} MutexSemaphoreIndex;

/**
 * @brief Indici per i semafori di controllo di ogni stazione.
 */
typedef enum {
    STATION_SEM_AVAILABLE_POSTS = 0, /**< Semaforo a conteggio per i posti operatore disponibili */
    STATION_SEM_USER_QUEUE,          /**< Sincronizzazione per l'attesa degli utenti alla stazione */
    STATION_SEM_REFILL_GATE,         /**< Cancello per bloccare servizo durante il rifornimento (0:Aperto, 1:Chiuso) */
    STATION_SEM_REFILL_ACK,          /**< Sincronizzazione (Appello) per il risveglio post-rifornimento */
    STATION_SEM_COUNT                /**< Totale semafori per stazione */
} StationSemaphoreIndex;

/**
 * @brief Rappresentazione di una stazione di distribuzione cibo.
 */
typedef struct {
    int message_queue_id;               /**< ID della coda di messaggi per gli ordini */
    int semaphore_set_id;               /**< ID del set di semafori della stazione (StationSemaphoreIndex) */
    int num_operators_assigned;         /**< Numero di operatori assegnati a questa stazione */
    int portions[MAX_DISHES_PER_CATEGORY];  /**< Disponibilità fisica delle porzioni di cibo */
} FoodDistributionStation;

/**
 * @brief Rappresentazione della stazione di pagamento (Cassa).
 */
typedef struct {
    int message_queue_id;               /**< ID della coda di messaggi per i pagamenti */
    int semaphore_set_id;               /**< ID del set di semafori (Cassa) */
    double daily_income;                /**< Incasso specifico della giornata corrente */
    double total_income;                /**< Incasso totale accumulato nella simulazione */
} CashierStation;

/**
 * @brief Area dedicata al consumo dei pasti (Refezione).
 */
typedef struct {
    int semaphore_set_id;               /**< ID Semaforo a conteggio per gestire i posti a sedere */
} DiningArea;

/**
 * @brief Struttura principale della Memoria Condivisa.
 * Contiene lo stato globale della simulazione accessibile a tutti i processi.
 */
struct MainSharedMemory {
    SimulationConfiguration configuration;    /**< Parametri di configurazione caricati dai file .conf */
    SimulationStatistics statistics;          /**< Statistiche globali aggiornate in tempo reale */
    SimulationMenu food_menu;                 /**< Menu della mensa con i nomi dei piatti */
    
    int shared_memory_id;               /**< ID della risorsa Shared Memory stessa */
    int semaphore_sync_id;              /**< ID Set Semafori Barriera (SyncBarrierIndex) */
    int semaphore_mutex_id;             /**< ID Set Semafori Mutex (MutexSemaphoreIndex) */
    int semaphore_ticket_id;            /**< ID Semaforo per la validazione ticket all'ingresso */

    pid_t process_group_pids[MAX_PROCESS_GROUPS]; /**< PGID dei vari gruppi di processi */

    FoodDistributionStation first_course_station;
    FoodDistributionStation second_course_station;
    FoodDistributionStation coffee_dessert_station;

    CashierStation register_station;
    DiningArea seat_area;

    int current_simulation_day;         /**< Giorno attuale della simulazione */
    int simulation_minutes_passed;      /**< Minuti simulati trascorsi dall'inizio del giorno */
    int is_simulation_running;          /**< Flag globale (1: Attiva, 0: Arresto Totale) */
    int current_simulation_status;      /**< Stato attuale (Aperto, In Chiusura, Disorder) */
};

typedef struct MainSharedMemory MainSharedMemory;

/**
 * @brief Collega il processo alla memoria condivisa della simulazione.
 * 
 * @param shared_memory_id ID della risorsa SHM (solitamente passato via argv).
 * @return MainSharedMemory* Puntatore all'area di memoria, termina il processo su errore.
 */
MainSharedMemory* attach_to_simulation_shared_memory(int shared_memory_id);

/**
 * @brief Esegue la pulizia di tutte le risorse IPC allocate.
 * @param shared_memory_ptr Puntatore alla struttura di memoria condivisa.
 */
void cleanup_ipc_resources(MainSharedMemory *shared_memory_ptr);

/**
 * @brief Termina la simulazione in modo controllato, pulendo le risorse.
 * @param shared_memory_ptr Puntatore alla memoria condivisa.
 * @param exit_code Codice di uscita per il processo principale.
 */
void terminate_simulation_gracefully(MainSharedMemory *shared_memory_ptr, int exit_code);

#endif
