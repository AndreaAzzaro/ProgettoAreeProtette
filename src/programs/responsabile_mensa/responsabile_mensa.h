/**
 * @file responsabile_mensa.h
 * @brief Header Master del Responsabile Mensa.
 * 
 * Questo file funge da Facade (Punto di accesso unico) che aggrega 
 * i prototipi degli altri sotto-moduli (setup_ipc, setup_population, simulation_engine).
 */

#ifndef RESPONSABILE_MENSA_H
#define RESPONSABILE_MENSA_H

#include "common.h"
#include "setup_ipc.h"
#include "setup_population.h"
#include "simulation_engine.h"
/* ==========================================================================
 *                          PROTOTIPI FACADE (Infrastruttura)
 * ========================================================================= */

/** @brief Carica i parametri dal file config.conf. Definizione in config.h */
SimulationConfiguration load_simulation_configuration(void);

/** @brief Carica i piatti dal file menu.conf. Definizione in menu.h */
SimulationMenu load_simulation_menu(void);

/** 
 * @brief Alloca e inizializza le risorse IPC iniziali (SHM, Sem, Code). 
 * Definizione in setup_ipc.h 
 */
MainSharedMemory* initialize_simulation_shared_memory(void);
void initialize_simulation_start_barriers(MainSharedMemory *shared_memory_ptr);
void initialize_daily_cycle_barriers(MainSharedMemory *shared_memory_ptr);
void initialize_global_simulation_mutexes(MainSharedMemory *shared_memory_ptr);
void initialize_food_distribution_station_semaphores(MainSharedMemory *shared_memory_ptr);
void initialize_dining_area_seats_semaphores(MainSharedMemory *shared_memory_ptr);
void initialize_ticket_validation_semaphores(MainSharedMemory *shared_memory_ptr);
void initialize_food_distribution_order_queues(MainSharedMemory *shared_memory_ptr);
void initialize_cashier_checkout_message_queue(MainSharedMemory *shared_memory_ptr);
void initialize_ipc_sources(MainSharedMemory *shared_memory_ptr);
void setup_worker_distribution(MainSharedMemory *shm_ptr);
void setup_prework_barrier(MainSharedMemory *shm_ptr);
void synchronize_prework_barrier(MainSharedMemory *shm_ptr);
void start_simulation(MainSharedMemory *shm_ptr);

/* ==========================================================================
 *                          PROTOTIPI FACADE (Popolazione)
 * ========================================================================= */

/** @brief Calcola matematicamente la distribuzione dei lavoratori. Definizione in setup_population.h */
void calculate_worker_distribution(int total_workers, int *average_times_array, int *distribution_results_array, int stations_count);

/** @brief Esegue il lancio dei processi operatori. Definizione in setup_population.h */
void launch_simulation_operators(MainSharedMemory *shared_memory_ptr);

/** @brief Esegue lo spawn dei processi utente. Definizione in setup_population.h */
void launch_simulation_users(MainSharedMemory *shared_memory_ptr, int users_to_launch_count);

/* ==========================================================================
 *                          PROTOTIPI FACADE (Motore)
 * ========================================================================= */

/** @brief Gestisce il loop dei giorni di simulazione. Definizione in simulation_engine.h */
void run_simulation_loop(MainSharedMemory *shm);

/** @brief Gestisce le operazioni di inizio giornata. Definizione in simulation_engine.h */
void handle_new_day(MainSharedMemory *shm);

/** @brief Gestisce il ciclo di rifornimento dei piatti. Definizione in simulation_engine.h */
void handle_refill_cycle(MainSharedMemory *shm);

#endif
