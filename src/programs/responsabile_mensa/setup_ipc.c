/**
 * @file setup_ipc.c
 * @brief Implementazione dell'inizializzazione delle risorse IPC.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "common.h"
#include "sem.h"
#include "shm.h"
#include "queue.h"
#include "utils.h"
#include "setup_ipc.h"

struct MainSharedMemory* initialize_simulation_shared_memory(int group_pool_size) {
    int shmid;
    struct MainSharedMemory *shm_ptr;
    /* Somma la parte fissa della struct alla parte variabile (Flexible Array Member) */
    size_t shm_size = sizeof(struct MainSharedMemory) + (group_pool_size * sizeof(GroupStatus));

    key_t key = ftok(IPC_KEY_PATH, IPC_PROJECT_ID);
    if (key == -1) {
        perror("[ERROR] ftok fallita per la memoria condivisa");
        exit(EXIT_FAILURE);
    }

    shmid = create_shared_memory_segment(key, shm_size, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1) {
        if (errno == EEXIST) {
            /* Se esiste già, lo rimuoviamo (presumibilmente rimasuglio di un crash precedente) */
            int old_shmid = shmget(key, 0, 0);
            if (old_shmid != -1) shmctl(old_shmid, IPC_RMID, NULL);
            shmid = create_shared_memory_segment(key, shm_size, IPC_CREAT | 0666);
        }
        
        if (shmid == -1) {
            perror("[ERROR] Creazione memoria condivisa fallita");
            exit(EXIT_FAILURE);
        }
    }

    shm_ptr = (struct MainSharedMemory *)attach_shared_memory_segment(shmid, false);
    if (shm_ptr == NULL) {
        perror("[ERROR] Attach memoria condivisa fallito");
        exit(EXIT_FAILURE);
    }

    /* Pulizia iniziale della memoria */
    memset(shm_ptr, 0, shm_size);
    shm_ptr->shared_memory_id = shmid;
    shm_ptr->group_pool_size = group_pool_size;
    shm_ptr->is_simulation_running = 1;
    shm_ptr->master_pid = getpid();

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
    init_sem_val(semid, MUTEX_ADD_USERS_PERMISSION, 0); /* Chiama V() il Direttore, P() add_users */
    
    shared_memory_ptr->semaphore_mutex_id = semid;
}

static void init_station_resource(FoodDistributionStation *station) {
    station->message_queue_id = create_message_queue(IPC_PRIVATE, IPC_CREAT | 0666);
    if (station->message_queue_id == -1) {
        perror("[ERROR] Creazione coda messaggi stazione fallita");
        exit(EXIT_FAILURE);
    }

    /* Ottimizzazione code per carichi elevati (es. 500 utenti) */
    struct msqid_ds ds;
    if (msgctl(station->message_queue_id, IPC_STAT, &ds) != -1) {
        ds.msg_qbytes = 65536; /* 64KB */
        msgctl(station->message_queue_id, IPC_SET, &ds);
    }

    station->semaphore_set_id = create_sem_set(IPC_PRIVATE, STATION_SEM_COUNT, IPC_CREAT | 0666);
    if (station->semaphore_set_id == -1) {
        perror("[ERROR] Creazione set semafori stazione fallita");
        exit(EXIT_FAILURE);
    }

    /* Inizializzazione semafori stazione */
    init_sem_val(station->semaphore_set_id, STATION_SEM_AVAILABLE_POSTS, 0); 
    init_sem_val(station->semaphore_set_id, STATION_SEM_USER_QUEUE, 0);
    init_sem_val(station->semaphore_set_id, STATION_SEM_REFILL_GATE, 0); 
    init_sem_val(station->semaphore_set_id, STATION_SEM_REFILL_ACK, 0);
}

void initialize_distribution_stations(struct MainSharedMemory *shared_memory_ptr) {
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


void initialize_cashier_checkout_message_queue(struct MainSharedMemory *shared_memory_ptr) {
    /* Creazione set semafori per la stazione Cassa */
    shared_memory_ptr->register_station.semaphore_set_id = create_sem_set(IPC_PRIVATE, STATION_SEM_COUNT, IPC_CREAT | 0666);
    if (shared_memory_ptr->register_station.semaphore_set_id == -1) {
        perror("[ERROR] Creazione set semafori cassa fallita");
        exit(EXIT_FAILURE);
    }

    /* Inizializzazione: posti disponibili verrà impostato dopo la distribuzione operatori */
    init_sem_val(shared_memory_ptr->register_station.semaphore_set_id, STATION_SEM_AVAILABLE_POSTS, 0);
    init_sem_val(shared_memory_ptr->register_station.semaphore_set_id, STATION_SEM_USER_QUEUE, 0);
    init_sem_val(shared_memory_ptr->register_station.semaphore_set_id, STATION_SEM_REFILL_GATE, 0);
    init_sem_val(shared_memory_ptr->register_station.semaphore_set_id, STATION_SEM_REFILL_ACK, 0);
    init_sem_val(shared_memory_ptr->register_station.semaphore_set_id, STATION_SEM_STOP_GATE, 0); /* 0=Cassa attiva */

    /* Creazione coda messaggi per la Cassa */
    shared_memory_ptr->register_station.message_queue_id = create_message_queue(IPC_PRIVATE, IPC_CREAT | 0666);
    if (shared_memory_ptr->register_station.message_queue_id == -1) {
        perror("[ERROR] Creazione coda messaggi cassa fallita");
        exit(EXIT_FAILURE);
    }

    /* Ottimizzazione cassa */
    struct msqid_ds ds;
    if (msgctl(shared_memory_ptr->register_station.message_queue_id, IPC_STAT, &ds) != -1) {
        ds.msg_qbytes = 65536;
        msgctl(shared_memory_ptr->register_station.message_queue_id, IPC_SET, &ds);
    }
}

void initialize_control_structures(struct MainSharedMemory *shm_ptr) {
    /* Inizializza il contatore utenti con il valore di configurazione */
    shm_ptr->current_total_users = shm_ptr->configuration.quantities.number_of_initial_users;
    shm_ptr->add_users_flag = 0;
    
    /* Crea la coda di controllo per add_users */
    shm_ptr->control_queue_id = create_message_queue(IPC_PRIVATE, IPC_CREAT | 0666);
    if (shm_ptr->control_queue_id == -1) {
        perror("[ERROR] Creazione coda di controllo fallita");
        exit(EXIT_FAILURE);
    }

    /* Ottimizzazione controllo */
    struct msqid_ds ds;
    if (msgctl(shm_ptr->control_queue_id, IPC_STAT, &ds) != -1) {
        ds.msg_qbytes = 16384; /* Controllo meno pesante */
        msgctl(shm_ptr->control_queue_id, IPC_SET, &ds);
    }
}

void initialize_group_sync_pool(struct MainSharedMemory *shm_ptr, int pool_size) {
    int total_sems = pool_size * GROUP_SEMS_PER_ENTRY;
    int semid = create_sem_set(IPC_PRIVATE, total_sems, IPC_CREAT | 0666);
    
    if (semid == -1) {
        perror("[ERROR] Creazione pool semafori gruppi fallita");
        exit(EXIT_FAILURE);
    }

    /* Inizializzazione: 
     * - PRE_CASHIER e EXIT a 0 
     * - TABLE_GATE a 1 (chiuso)
     */
    for (int i = 0; i < pool_size; i++) {
        int base = i * GROUP_SEMS_PER_ENTRY;
        init_sem_val(semid, base + GROUP_SEM_PRE_CASHIER, 0);
        init_sem_val(semid, base + GROUP_SEM_TABLE_GATE, 1);
        init_sem_val(semid, base + GROUP_SEM_EXIT, 0);
    }

    shm_ptr->group_sync_semaphore_id = semid;
    shm_ptr->group_pool_size = pool_size;
}

void initialize_ipc_sources(struct MainSharedMemory *shm_ptr) {
    initialize_simulation_start_barriers(shm_ptr);
    initialize_daily_cycle_barriers(shm_ptr);
    initialize_global_simulation_mutexes(shm_ptr);
    initialize_distribution_stations(shm_ptr);
    initialize_dining_area_seats_semaphores(shm_ptr);
    initialize_ticket_validation_semaphores(shm_ptr);
    initialize_cashier_checkout_message_queue(shm_ptr);
    initialize_control_structures(shm_ptr);
}
