/**
 * @file setup_ipc.c
 * @brief Implementazione dell'inizializzazione delle risorse IPC.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "sem.h"
#include "shm.h"
#include "queue.h"
#include "utils.h"
#include "setup_ipc.h"

struct MainSharedMemory* initialize_simulation_shared_memory(void) {
    int shmid;
    struct MainSharedMemory *shm_ptr;
    size_t shm_size = sizeof(struct MainSharedMemory);

    shmid = create_shared_memory_segment(IPC_PRIVATE, shm_size, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("[ERROR] Creazione memoria condivisa fallita");
        exit(EXIT_FAILURE);
    }

    shm_ptr = (struct MainSharedMemory *)attach_shared_memory_segment(shmid, false);
    if (shm_ptr == NULL) {
        perror("[ERROR] Attach memoria condivisa fallito");
        exit(EXIT_FAILURE);
    }

    /* Pulizia iniziale della memoria */
    memset(shm_ptr, 0, shm_size);
    shm_ptr->shared_memory_id = shmid;
    shm_ptr->is_simulation_running = 1;

    return shm_ptr;
}

void initialize_simulation_start_barriers(struct MainSharedMemory *shared_memory_ptr) {
    int semid = create_sem_set(IPC_PRIVATE, SYNC_BARRIER_SEM_COUNT, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("[ERROR] Creazione barriere di sincronizzazione fallita");
        exit(EXIT_FAILURE);
    }
    shared_memory_ptr->semaphore_sync_id = semid;
}

void initialize_daily_cycle_barriers(struct MainSharedMemory *shared_memory_ptr) {
    int semid = shared_memory_ptr->semaphore_sync_id;
    
    /* Inizializzo i semafori della barriera a 0. 
     * NOTA: Le barriere verranno preparate (setup_barrier) all'inizio di ogni ciclo 
     * dall'engine della simulazione. 
     */
    for (int i = 0; i < SYNC_BARRIER_SEM_COUNT; i++) {
        init_sem_val(semid, i, 0);
    }
}

void initialize_global_simulation_mutexes(struct MainSharedMemory *shared_memory_ptr) {
    int semid = create_sem_set(IPC_PRIVATE, MUTEX_SEMAPHORE_COUNT, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("[ERROR] Creazione mutex globali fallita");
        exit(EXIT_FAILURE);
    }
    
    init_sem_val(semid, MUTEX_SIMULATION_STATS, 1);
    init_sem_val(semid, MUTEX_SHARED_DATA, 1);
    
    shared_memory_ptr->semaphore_mutex_id = semid;
}

static void init_station_resource(FoodDistributionStation *station) {
    station->message_queue_id = create_message_queue(IPC_PRIVATE, IPC_CREAT | 0666);
    if (station->message_queue_id == -1) {
        perror("[ERROR] Creazione coda messaggi stazione fallita");
        exit(EXIT_FAILURE);
    }

    station->semaphore_set_id = create_sem_set(IPC_PRIVATE, STATION_SEM_COUNT, IPC_CREAT | 0666);
    if (station->semaphore_set_id == -1) {
        perror("[ERROR] Creazione set semafori stazione fallita");
        exit(EXIT_FAILURE);
    }

    /* Inizializzazione semafori stazione */
    init_sem_val(station->semaphore_set_id, STATION_SEM_AVAILABLE_POSTS, 0); /* Verrà impostato dal numero di operatori */
    init_sem_val(station->semaphore_set_id, STATION_SEM_USER_QUEUE, 0);
    init_sem_val(station->semaphore_set_id, STATION_SEM_REFILL_GATE, 0); /* Aperto di default */
    init_sem_val(station->semaphore_set_id, STATION_SEM_REFILL_ACK, 0);
}

void initialize_food_distribution_station_semaphores(struct MainSharedMemory *shared_memory_ptr) {
    init_station_resource(&shared_memory_ptr->first_course_station);
    init_station_resource(&shared_memory_ptr->second_course_station);
    init_station_resource(&shared_memory_ptr->coffee_dessert_station);
}

void initialize_dining_area_seats_semaphores(struct MainSharedMemory *shared_memory_ptr) {
    int semid = create_sem_set(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("[ERROR] Creazione semaforo posti a sedere fallita");
        exit(EXIT_FAILURE);
    }
    
    /* Il valore verrà impostato dall'engine dopo il caricamento della config */
    init_sem_val(semid, 0, 0);
    shared_memory_ptr->seat_area.semaphore_set_id = semid;
}

void initialize_ticket_validation_semaphores(struct MainSharedMemory *shared_memory_ptr) {
    int semid = create_sem_set(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("[ERROR] Creazione semaforo ticket fallita");
        exit(EXIT_FAILURE);
    }
    
    init_sem_val(semid, 0, TICKET_VALIDATORS_COUNT);
    shared_memory_ptr->semaphore_ticket_id = semid;
}

void initialize_food_distribution_order_queues(struct MainSharedMemory *shared_memory_ptr) {
    /* Già fatto in init_station_resource() per ogni stazione */
}

void initialize_cashier_checkout_message_queue(struct MainSharedMemory *shared_memory_ptr) {
    shared_memory_ptr->register_station.message_queue_id = create_message_queue(IPC_PRIVATE, IPC_CREAT | 0666);
    if (shared_memory_ptr->register_station.message_queue_id == -1) {
        perror("[ERROR] Creazione coda messaggi cassa fallita");
        exit(EXIT_FAILURE);
    }
    
    /* La cassa non ha un set di semafori dedicato complesso come le stazioni, 
     * ma per coerenza potremmo inizializzarne uno se necessario. 
     */
}
