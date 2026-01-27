/**
 * @file setup_population.c
 * @brief Implementazione della creazione dei processi figli (Operatori e Utenti).
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "common.h"
#include "setup_population.h"
#include "utils.h"
#include "sem.h"

void calculate_worker_distribution(int total_workers, int *average_times_array, int *distribution_results_array, int stations_count) {
    int total_time = 0;
    for (int i = 0; i < stations_count; i++) {
        total_time += average_times_array[i];
    }

    if (total_time == 0) {
        /* Distribuzione uniforme se i tempi sono zero */
        for (int i = 0; i < stations_count; i++) {
            distribution_results_array[i] = total_workers / stations_count;
        }
    } else {
        int distributed = 0;
        for (int i = 0; i < stations_count; i++) {
            distribution_results_array[i] = (int)((double)total_workers * average_times_array[i] / total_time);
            distributed += distribution_results_array[i];
        }
        /* Gestione resto per approssimazione */
        distribution_results_array[0] += (total_workers - distributed);
    }
}

static void exec_worker(int shmid, int station_type) {
    char shm_str[20];
    char type_str[10];
    sprintf(shm_str, "%d", shmid);
    sprintf(type_str, "%d", station_type);
    
    execl("./bin/operatore", "operatore", shm_str, type_str, (char *)NULL);
    perror("[ERROR] execl operatore fallita");
    exit(EXIT_FAILURE);
}

void launch_simulation_operators(MainSharedMemory *shared_memory_ptr) {
    int shmid = shared_memory_ptr->shared_memory_id;
    
    /* 1. Lancio operatori per le 3 stazioni di cibo */
    int station_operators[] = {
        shared_memory_ptr->first_course_station.num_operators_assigned,
        shared_memory_ptr->second_course_station.num_operators_assigned,
        shared_memory_ptr->coffee_dessert_station.num_operators_assigned
    };
    int groups[] = { GROUP_FIRST_COURSES, GROUP_SECOND_COURSES, GROUP_DESSERT_COFFEE };
    
    for (int s = 0; s < 3; s++) {
        pid_t pgid = 0;
        for (int i = 0; i < station_operators[s]; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                setpgid(0, pgid);
                exec_worker(shmid, s);
            } else if (pid > 0) {
                if (i == 0) pgid = pid;
            }
        }
        shared_memory_ptr->process_group_pids[groups[s]] = pgid;
    }

    /* 2. Lancio operatori di cassa */
    pid_t cassa_pgid = 0;
    for (int i = 0; i < shared_memory_ptr->configuration.seats.seats_cash_desk; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            setpgid(0, cassa_pgid);
            char shm_str[20];
            sprintf(shm_str, "%d", shmid);
            execl("./bin/operatore_cassa", "operatore_cassa", shm_str, (char *)NULL);
            perror("[ERROR] execl operatore_cassa fallita");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            if (i == 0) cassa_pgid = pid;
        }
    }
    shared_memory_ptr->process_group_pids[GROUP_CASHIERS] = cassa_pgid;
}

static int planned_groups_count = 0;
static int *planned_group_sizes = NULL;

int calculate_initial_groups_count(MainSharedMemory *shm_ptr) {
    int users_to_assign = shm_ptr->configuration.quantities.number_of_initial_users;
    
    /* Alloca nel caso peggiore: 1 gruppo per ogni utente */
    planned_group_sizes = (int *)malloc(users_to_assign * sizeof(int));
    if (planned_group_sizes == NULL) {
        perror("[ERROR] Allocazione dinamica gruppi fallita");
        exit(EXIT_FAILURE);
    }

    planned_groups_count = 0;
    while (users_to_assign > 0) {
        int g_size = generate_random_integer(1, MAX_USERS_PER_GROUP);
        if (g_size > users_to_assign) g_size = users_to_assign;
        
        planned_group_sizes[planned_groups_count] = g_size;
        planned_groups_count++;
        users_to_assign -= g_size;
    }
    
    /* Aggiungiamo un margine per futuri add_users (es. 100 slot extra) */
    return planned_groups_count + 100;
}

void launch_simulation_users(MainSharedMemory *shared_memory_ptr) {
    int shmid = shared_memory_ptr->shared_memory_id;
    int current_sync_index = 0;

    printf("[MASTER] Lancio di %d gruppi di utenti...\n", planned_groups_count);

    for (int g = 0; g < planned_groups_count; g++) {
        int group_size = planned_group_sizes[g];

        /* [INIZIALIZZAZIONE GRUPPO] Imposta lo stato dinamico prima dello spawn dei membri */
        shared_memory_ptr->group_statuses[current_sync_index].active_members = group_size;
        shared_memory_ptr->group_statuses[current_sync_index].group_leader_pid = 0;

        for (int i = 0; i < group_size; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                /* Appartenenza al PGID globale degli Utenti per segnali broadcast */
                pid_t users_global_pgid = shared_memory_ptr->process_group_pids[GROUP_USERS];
                setpgid(0, users_global_pgid);

                char shm_str[24], gsize_str[24], gindex_str[24], is_leader_str[8];
                sprintf(shm_str, "%d", shmid);
                sprintf(gsize_str, "%d", group_size);
                sprintf(gindex_str, "%d", current_sync_index);
                sprintf(is_leader_str, "%d", (i == 0)); /* Il primo del gruppo è il leader */

                execl("./bin/utente", "utente", shm_str, gsize_str, gindex_str, is_leader_str, (char *)NULL);
                perror("[ERROR] execl utente fallita");
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                /* Registrazione del processo nel registro del Master per la gestione dei deadlock */
                reserve_sem(shared_memory_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);
                for (int r = 0; r < MAX_USERS_REGISTRY; r++) {
                    if (shared_memory_ptr->user_registry[r].pid == 0) {
                        shared_memory_ptr->user_registry[r].pid = pid;
                        shared_memory_ptr->user_registry[r].group_index = current_sync_index;
                        break;
                    }
                }
                release_sem(shared_memory_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);

                /* Il Master stabilisce il PGID globale basandosi sul primo utente in assoluto */
                if (shared_memory_ptr->process_group_pids[GROUP_USERS] == 0) {
                    shared_memory_ptr->process_group_pids[GROUP_USERS] = pid;
                }
            }
        }
        current_sync_index++;
    }

    /* Libera la memoria allocata dinamicamente */
    free(planned_group_sizes);
    planned_group_sizes = NULL;
}

void setup_worker_distribution(MainSharedMemory *shm_ptr) {
    int total_workers = shm_ptr->configuration.quantities.number_of_workers;
    int stations_count = 3; 
    int average_times[3];
    int worker_results[3];

    /* Recupero i tempi medi dai campi specifici della configurazione */
    average_times[0] = shm_ptr->configuration.timings.average_service_time_primi;
    average_times[1] = shm_ptr->configuration.timings.average_service_time_secondi;
    average_times[2] = shm_ptr->configuration.timings.average_service_time_coffee;

    calculate_worker_distribution(total_workers, average_times, worker_results, stations_count);

    shm_ptr->first_course_station.num_operators_assigned = worker_results[0];
    shm_ptr->second_course_station.num_operators_assigned = worker_results[1];
    shm_ptr->coffee_dessert_station.num_operators_assigned = worker_results[2];
}

void initialize_station_operator_semaphores(MainSharedMemory *shm_ptr) {
    /* Inizializza il numero di posti operatore disponibili per ogni stazione */
    init_sem_val(shm_ptr->first_course_station.semaphore_set_id, 
                 STATION_SEM_AVAILABLE_POSTS, 
                 shm_ptr->first_course_station.num_operators_assigned);
    
    init_sem_val(shm_ptr->second_course_station.semaphore_set_id, 
                 STATION_SEM_AVAILABLE_POSTS, 
                 shm_ptr->second_course_station.num_operators_assigned);
    
    init_sem_val(shm_ptr->coffee_dessert_station.semaphore_set_id, 
                 STATION_SEM_AVAILABLE_POSTS, 
                 shm_ptr->coffee_dessert_station.num_operators_assigned);
    
    /* Cassa: usa il numero di cassieri dalla configurazione */
    init_sem_val(shm_ptr->register_station.semaphore_set_id, 
                 STATION_SEM_AVAILABLE_POSTS, 
                 shm_ptr->configuration.seats.seats_cash_desk);

    /* Posti Tavolo (DiningArea): usa NOFTABLESEATS dalla configurazione */
    init_sem_val(shm_ptr->seat_area.semaphore_set_id, 
                 0, 
                 shm_ptr->configuration.seats.total_dining_seats);
}
