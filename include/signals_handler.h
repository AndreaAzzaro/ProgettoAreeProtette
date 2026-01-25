#ifndef SIGNALS_HANDLER_H
#define SIGNALS_HANDLER_H

#include <signal.h>

#include "common.h"

/**
 * @brief Imposta i gestori dei segnali per il processo Direttore.
 * @param shm Puntatore alla memoria condivisa per permettere la pulizia.
 */
void setup_director_signals(SharedMemory *shm);

// Flag globale per gestire le richieste di rifornimento (Innescata da SIGUSR2)
extern volatile sig_atomic_t refill_needed;

void terminate_simulation(SharedMemory *shm, int exit_code);

/**
 * @brief Imposta i gestori dei segnali per i processi figli (Operatori/Utenti).
 * @param sim_running_flag Puntatore alla flag globale della simulazione.
 * @param day_running_flag Puntatore alla flag specifica della giornata attuale.
 */
void setup_child_signals(volatile sig_atomic_t *sim_running_flag, volatile sig_atomic_t *day_running_flag);

#endif
