/**
 * @file simulation_engine.c
 * @brief Implementazione minimale del motore di simulazione (Punto Zero).
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "simulation_engine.h"

void run_simulation_loop(MainSharedMemory *shm) {
    printf("[MASTER] Engine avviato correttamente. In attesa di istruzioni operative...\n");

    /* Loop minimale di attesa terminazione */
    while (shm->is_simulation_running) {
        sleep(1); 
    }

    printf("[MASTER] Engine in chiusura.\n");
}
