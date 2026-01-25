/**
 * @file setup_ipc.h
 * @brief Definizioni per l'inizializzazione delle risorse IPC del Responsabile Mensa.
 * 
 * Questo modulo si occupa della configurazione granulare di semafori, 
 * code di messaggi e memoria condivisa necessari per la simulazione.
 */

#ifndef SETUP_IPC_H
#define SETUP_IPC_H

/** Forward declaration: evita di includere common.h nell'header */
struct MainSharedMemory;

/**
 * @brief Alloca e inizializza il segmento principale di memoria condivisa.
 * 
 * @return struct MainSharedMemory* Puntatore all'area di memoria collegata.
 */
struct MainSharedMemory* initialize_simulation_shared_memory(void);

/* ==========================================================================
 *                 INIZIALIZZAZIONE SEMAFORI (System V)
 * ========================================================================== */

/**
 * @brief Inizializza le barriere di sincronizzazione (Start Barrier).
 * 
 * @param shared_memory_ptr Puntatore alla memoria condivisa principale.
 */
void initialize_simulation_start_barriers(struct MainSharedMemory *shared_memory_ptr);

/**
 * @brief Inizializza le barriere di sincronizzazione per l'avvio e la fine della giornata.
 * 
 * Implementa il pattern "Ping-Pong" per gestire i giorni successivi al primo.
 * @param shared_memory_ptr Puntatore alla memoria condivisa principale.
 */
void initialize_daily_cycle_barriers(struct MainSharedMemory *shared_memory_ptr);

/**
 * @brief Inizializza i semafori Mutex per la protezione delle sezioni critiche globali.
 * 
 * @param shared_memory_ptr Puntatore alla memoria condivisa principale.
 */
void initialize_global_simulation_mutexes(struct MainSharedMemory *shared_memory_ptr);

/**
 * @brief Inizializza i semafori a conteggio per le postazioni delle stazioni di distribuzione.
 * 
 * @param shared_memory_ptr Puntatore alla memoria condivisa principale.
 */
void initialize_food_distribution_station_semaphores(struct MainSharedMemory *shared_memory_ptr);

/**
 * @brief Inizializza il semaforo a conteggio per gestire i posti a sedere nell'area refezione.
 * 
 * @param shared_memory_ptr Puntatore alla memoria condivisa principale.
 */
void initialize_dining_area_seats_semaphores(struct MainSharedMemory *shared_memory_ptr);

/**
 * @brief Inizializza il semaforo per il controllo dei flussi degli utenti con/senza ticket.
 * 
 * @param shared_memory_ptr Puntatore alla memoria condivisa principale.
 */
void initialize_ticket_validation_semaphores(struct MainSharedMemory *shared_memory_ptr);

/* ==========================================================================
 *                 INIZIALIZZAZIONE CODE DI MESSAGGI
 * ========================================================================== */

/**
 * @brief Crea e inizializza le code di messaggi per gli ordini nelle stazioni di cibo.
 * 
 * @param shared_memory_ptr Puntatore alla memoria condivisa principale.
 */
void initialize_food_distribution_order_queues(struct MainSharedMemory *shared_memory_ptr);

/**
 * @brief Crea la coda di messaggi per la gestione dei pagamenti alla cassa.
 * 
 * @param shared_memory_ptr Puntatore alla memoria condivisa principale.
 */
void initialize_cashier_checkout_message_queue(struct MainSharedMemory *shared_memory_ptr);

#endif
