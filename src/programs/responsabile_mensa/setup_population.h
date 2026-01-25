#ifndef SETUP_POPULATION_H
#define SETUP_POPULATION_H

#include "common.h"

/**
 * @brief Calcola matematicamente la distribuzione dei lavoratori.
 */
void calculate_worker_distribution(int total_workers, int *times, int *dist, int sizeDist);

/**
 * @brief Esegue fork ed exec per tutti gli operatori (Cucina e Cassa).
 */
void launch_operators(SharedMemory *shm);

/**
 * @brief Funzione interna per il lancio effettivo di un numero N di utenti.
 */
void spawn_users(SharedMemory *shm, int count);

#endif
