/**
 * @file utente.c
 * @brief Processo Utente per test barriera startup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "utente.h"
#include "signals_handler.h"
#include "sem.h"

int main(int argc, char *argv[]) {
    StatoUtente utente;
    
    if (argc < 2) exit(EXIT_FAILURE);
    utente.shared_memory_id = atoi(argv[1]);
    utente.shm_ptr = attach_to_simulation_shared_memory(utente.shared_memory_id);

    /* Sincronizzazione di Startup Globale */
    sync_child_start(utente.shm_ptr->semaphore_sync_id, BARRIER_STARTUP_READY, BARRIER_STARTUP_GATE);
    printf("[DEBUG] Utente PID %d: Barriera startup superata.\n", getpid());

    return 0;
}
