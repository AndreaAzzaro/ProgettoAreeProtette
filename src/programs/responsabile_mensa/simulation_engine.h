#ifndef SIMULATION_ENGINE_H
#define SIMULATION_ENGINE_H

#include "common.h"

/**
 * @brief Avvia il ciclo principale della simulazione per tutti i giorni previsti.
 * Coordina l'inizio di ogni giornata e il timing globale.
 * @param shm Puntatore alla memoria condivisa.
 */
void run_simulation_loop(SharedMemory *shm);

/**
 * @brief Gestisce le operazioni di inizio giornata (reset statistiche giornaliere, 
 * incremento contatore giorno, etc.).
 * @param shm Puntatore alla memoria condivisa.
 */
void handle_new_day(SharedMemory *shm);

/**
 * @brief Gestisce il ciclo di rifornimento dei piatti nelle stazioni di distribuzione.
 * @param shm Puntatore alla memoria condivisa.
 */
void handle_refill_cycle(SharedMemory *shm);

#endif
