#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "../../include/common.h"
#include "../../include/sem.h"

/* ==========================================================================
 *                          FUNZIONI PUBBLICHE
 * ========================================================================= */

void cleanup_ipc_resources(MainSharedMemory *shared_memory_ptr) {
    if (shared_memory_ptr == NULL) {
        return;
    }

    /* 1. Eliminazione Semafori Globali */
    if (shared_memory_ptr->semaphore_sync_id != -1) {
        delete_sem_set(shared_memory_ptr->semaphore_sync_id);
    }
    if (shared_memory_ptr->semaphore_mutex_id != -1) {
        delete_sem_set(shared_memory_ptr->semaphore_mutex_id);
    }
    if (shared_memory_ptr->semaphore_ticket_id != -1) {
        delete_sem_set(shared_memory_ptr->semaphore_ticket_id);
    }

    /* 2. Eliminazione Risorse Stazioni di Distribuzione */
    /* Primi Piatti */
    if (shared_memory_ptr->first_course_station.message_queue_id != -1) {
        msgctl(shared_memory_ptr->first_course_station.message_queue_id, IPC_RMID, NULL);
    }
    if (shared_memory_ptr->first_course_station.semaphore_set_id != -1) {
        delete_sem_set(shared_memory_ptr->first_course_station.semaphore_set_id);
    }

    /* Secondi Piatti */
    if (shared_memory_ptr->second_course_station.message_queue_id != -1) {
        msgctl(shared_memory_ptr->second_course_station.message_queue_id, IPC_RMID, NULL);
    }
    if (shared_memory_ptr->second_course_station.semaphore_set_id != -1) {
        delete_sem_set(shared_memory_ptr->second_course_station.semaphore_set_id);
    }

    /* Bar e Dolci */
    if (shared_memory_ptr->coffee_dessert_station.message_queue_id != -1) {
        msgctl(shared_memory_ptr->coffee_dessert_station.message_queue_id, IPC_RMID, NULL);
    }
    if (shared_memory_ptr->coffee_dessert_station.semaphore_set_id != -1) {
        delete_sem_set(shared_memory_ptr->coffee_dessert_station.semaphore_set_id);
    }

    /* 3. Eliminazione Risorse Cassa e Area Refezione */
    if (shared_memory_ptr->register_station.message_queue_id != -1) {
        msgctl(shared_memory_ptr->register_station.message_queue_id, IPC_RMID, NULL);
    }
    if (shared_memory_ptr->register_station.semaphore_set_id != -1) {
        delete_sem_set(shared_memory_ptr->register_station.semaphore_set_id);
    }
    if (shared_memory_ptr->seat_area.semaphore_set_id != -1) {
        delete_sem_set(shared_memory_ptr->seat_area.semaphore_set_id);
    }

    /* 4. Eliminazione finale della Shared Memory */
    /* Nota: Usiamo una variabile locale per l'ID per evitare race conditions durante la detaching */
    int shared_memory_resource_id = shared_memory_ptr->shared_memory_id;
    if (shared_memory_resource_id != -1) {
        shmctl(shared_memory_resource_id, IPC_RMID, NULL);
    }
}

void terminate_simulation_gracefully(MainSharedMemory *shared_memory_ptr, int exit_code) {
    if (shared_memory_ptr != NULL) {
        printf("\n[SISTEMA] Avvio procedura di terminazione controllata...\n");
        cleanup_ipc_resources(shared_memory_ptr);
        printf("[SISTEMA] Risorse IPC rimosse correttamente. Chiusura simulazione.\n");
    }
    
    exit(exit_code);
}
