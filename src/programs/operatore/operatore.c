/**
 * @file operatore.c
 * @brief Processo Operatore per test barriera startup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "operatore.h"
#include "signals_handler.h"
#include "sem.h"

int main(int argc, char *argv[]) {
    StatoOperatore operatore;
    
    if (argc < 3) exit(EXIT_FAILURE);
    operatore.shared_memory_id = atoi(argv[1]);
    operatore.station_type = atoi(argv[2]);
    operatore.shm_ptr = attach_to_simulation_shared_memory(operatore.shared_memory_id);

    /* Sincronizzazione di Startup */
    sync_child_start(operatore.shm_ptr->semaphore_sync_id, BARRIER_STARTUP_READY, BARRIER_STARTUP_GATE);
    printf("[DEBUG] Operatore PID %d: Barriera startup superata.\n", getpid());

    return 0;
}
