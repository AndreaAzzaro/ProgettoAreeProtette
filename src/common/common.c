/**
 * @file common.c
 * @brief Funzioni comuni per la gestione della memoria condivisa e cleanup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "shm.h"
#include "sem.h"
#include "queue.h"

MainSharedMemory* attach_to_simulation_shared_memory(int shared_memory_id) {
    MainSharedMemory *shm_ptr = (MainSharedMemory *)attach_shared_memory_segment(shared_memory_id, false);
    if (shm_ptr == NULL) {
        perror("[ERROR] Impossibile collegarsi alla memoria condivisa");
        exit(EXIT_FAILURE);
    }
    return shm_ptr;
}

void cleanup_ipc_resources(MainSharedMemory *shared_memory_ptr) {
    if (!shared_memory_ptr) return;

    /* 1. Code messaggi stazioni */
    remove_message_queue(shared_memory_ptr->first_course_station.message_queue_id);
    remove_message_queue(shared_memory_ptr->second_course_station.message_queue_id);
    remove_message_queue(shared_memory_ptr->coffee_dessert_station.message_queue_id);
    remove_message_queue(shared_memory_ptr->register_station.message_queue_id);

    /* 2. Set semafori stazioni */
    delete_sem_set(shared_memory_ptr->first_course_station.semaphore_set_id);
    delete_sem_set(shared_memory_ptr->second_course_station.semaphore_set_id);
    delete_sem_set(shared_memory_ptr->coffee_dessert_station.semaphore_set_id);

    /* 3. Risorse globali */
    delete_sem_set(shared_memory_ptr->semaphore_sync_id);
    delete_sem_set(shared_memory_ptr->semaphore_mutex_id);
    delete_sem_set(shared_memory_ptr->semaphore_ticket_id);
    delete_sem_set(shared_memory_ptr->seat_area.semaphore_set_id);

    /* 4. Memoria condivisa */
    int shmid = shared_memory_ptr->shared_memory_id;
    detach_shared_memory_segment(shared_memory_ptr);
    remove_shared_memory_segment(shmid);
}

void terminate_simulation_gracefully(MainSharedMemory *shared_memory_ptr, int exit_code) {
    printf("\n[SYSTEM] Terminazione simulazione in corso...\n");
    cleanup_ipc_resources(shared_memory_ptr);
    exit(exit_code);
}
