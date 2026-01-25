#ifndef RESPONSABILE_MENSA_H
#define RESPONSABILE_MENSA_H

#include "common.h"
#include "setup_ipc.h"
#include "setup_population.h"
#include "simulation_engine.h"

/**
 * @file responsabile_mensa.h
 * @brief Header Master del Responsabile Mensa.
 * Aggrega tutti i moduli necessari per gestire la simulazione.
 */

/* --- FASE 1: Inizializzazione (responsabile_mensa.c / setup_ipc.c) --- */
SimConfig load_SimConfig();
Menu load_Menu();
int setup_shm();
void setup_semaphores(SharedMemory *shm);
void setup_queues(SharedMemory *shm);
void populate_shared_memory(SharedMemory *shm, int shmid, SimConfig cfg, Menu m);

/* --- FASE 2: Popolazione (setup_population.c) --- */
void launch_operators(SharedMemory *shm);
void launch_users(SharedMemory *shm);
void start_simulation_barrier(SharedMemory *shm);

/* --- FASE 3: Ciclo di Simulazione (Motore) --- */
void run_simulation_loop(SharedMemory *shm);
void handle_new_day(SharedMemory *shm);
void handle_refill_cycle(SharedMemory *shm);

/* --- FASE 4: Chiusura e Cleanup (common.c / responsabile_mensa.c) --- */
void collect_final_stats(SharedMemory *shm);
void terminate_simulation(SharedMemory *shm, int exit_code);
void cleanup_ipc(SharedMemory *shm);

#endif
