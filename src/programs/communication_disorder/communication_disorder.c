/**
 * @file communication_disorder.c
 * @brief Utility esterna per bloccare temporaneamente le casse (Communication Disorder).
 * 
 * Questo processo interrompe il funzionamento dei pagamenti automatici per STOP_DURATION
 * secondi, bloccando il funzionamento della stazione cassa.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "common.h"
#include "shm.h"
#include "sem.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    printf("[DISORDER] Communication Disorder in avvio...\n");

    /* 1. Generazione chiave deterministica tramite ftok() */
    key_t key = ftok(IPC_KEY_PATH, IPC_PROJECT_ID);
    if (key == -1) {
        perror("[ERROR] ftok fallita. Assicurati che config/config.conf esista");
        return EXIT_FAILURE;
    }

    /* 2. Recupero shmid dal kernel */
    int shmid = shmget(key, 0, 0);
    if (shmid == -1) {
        fprintf(stderr, "[ERROR] Impossibile trovare la memoria condivisa della simulazione.\n");
        fprintf(stderr, "La simulazione è stata avviata?\n");
        return EXIT_FAILURE;
    }

    /* 3. Connessione alla memoria condivisa */
    MainSharedMemory *shm = attach_to_simulation_shared_memory(shmid);
    
    /* 4. Lettura STOP_DURATION dalla configurazione */
    int stop_duration = shm->configuration.timings.stop_duration_minutes;
    printf("[DISORDER] Durata blocco casse: %d secondi simulati.\n", stop_duration);

    /* 5. Blocco delle casse: alza il gate (semaforo da 0 a 1) */
    printf("[DISORDER] Attivazione blocco casse...\n");
    release_sem(shm->register_station.semaphore_set_id, STATION_SEM_STOP_GATE);
    
    printf("[DISORDER] Casse BLOCCATE. Attesa di %d secondi...\n", stop_duration);

    /* 6. Attesa del tempo configurato (usando tempo reale per semplicità) */
    sleep(stop_duration);

    /* 7. Ripristino delle casse: riabbassa il gate (semaforo da 1 a 0) */
    printf("[DISORDER] Ripristino funzionamento casse...\n");
    reserve_sem(shm->register_station.semaphore_set_id, STATION_SEM_STOP_GATE);
    
    printf("[DISORDER] Casse RIPRISTINATE. Terminazione utility.\n");

    /* 8. Detach dalla memoria condivisa */
    detach_shared_memory_segment(shm);
    
    return EXIT_SUCCESS;
}
