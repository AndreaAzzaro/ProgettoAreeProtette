#ifndef SIMULATION_ENGINE_H
#define SIMULATION_ENGINE_H

#include "common.h"

/**
 * @brief Avvia il ciclo principale della simulazione per tutti i giorni previsti.
 * Coordina l'inizio di ogni giornata e il timing globale.
 * @param shm Puntatore alla memoria condivisa.
 */
void run_simulation_loop(MainSharedMemory *shm);

/**
 * @brief Configura e attiva il timer POSIX per la durata della giornata.
 * @param shm Puntatore alla memoria condivisa.
 */
void arm_daily_timer(MainSharedMemory *shm);

/**
 * @brief Gestisce il ciclo di rifornimento dei piatti nelle stazioni di distribuzione.
 * @param shm Puntatore alla memoria condivisa.
 */
void handle_refill_cycle(MainSharedMemory *shm);

/**
 * @brief Configura l'handler per gestire la terminazione asincrona dei figli.
 * @param shm Puntatore alla memoria condivisa.
 */
void setup_sigchld_handler(MainSharedMemory *shm);

/**
 * @brief Configura i segnali per la chiusura della giornata o interruzione (SIGINT, SIGUSR1).
 * @param shm Puntatore alla memoria condivisa.
 */
void setup_signal_close_day(MainSharedMemory *shm);

/**
 * @brief Configura il timer per il rifornimento casuale delle stazioni.
 */
void setup_refill_signal(void);

/**
 * @brief Invia un segnale a tutti i gruppi di processi registrati in SHM.
 * @param shm Puntatore alla memoria condivisa.
 * @param signal Il segnale da inviare.
 */
void broadcast_signal_to_all_groups(MainSharedMemory *shm, int signal);

/**
 * @brief Inizializza o resetta le barriere di sincronizzazione dei gruppi amici.
 * @param shm_ptr Puntatore alla memoria condivisa.
 */
void setup_group_barriers(MainSharedMemory *shm_ptr);


#endif
