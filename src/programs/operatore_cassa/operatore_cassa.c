/**
 * @file operatore_cassa.c
 * @brief Processo Cassiere per test barriera startup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "operatore_cassa.h"
#include "signals_handler.h"
#include "sem.h"

int main(int argc, char *argv[]) {
    StatoCassiere cassiere;
    
    if (argc < 2) exit(EXIT_FAILURE);
    cassiere.shared_memory_id = atoi(argv[1]);
    cassiere.shm_ptr = attach_to_simulation_shared_memory(cassiere.shared_memory_id);

    /* Sincronizzazione di Startup */
    sync_child_start(cassiere.shm_ptr->semaphore_sync_id, BARRIER_STARTUP_READY, BARRIER_STARTUP_GATE);
    printf("[DEBUG] Cassiere PID %d: Barriera startup superata.\n", getpid());

    return 0;
}
