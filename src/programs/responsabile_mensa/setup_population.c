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

void launch_simulation_users(MainSharedMemory *shared_memory_ptr) {
    int shmid = shared_memory_ptr->shared_memory_id;
    int users_to_launch_count = shared_memory_ptr->configuration.quantities.number_of_initial_users;
    pid_t users_pgid = shared_memory_ptr->process_group_pids[GROUP_USERS];

    for (int i = 0; i < users_to_launch_count; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            setpgid(0, users_pgid);
            char shm_str[20];
            sprintf(shm_str, "%d", shmid);
            execl("./bin/utente", "utente", shm_str, (char *)NULL);
            perror("[ERROR] execl utente fallita");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            if (users_pgid == 0) {
                users_pgid = pid;
                shared_memory_ptr->process_group_pids[GROUP_USERS] = users_pgid;
            }
        }
    }
}
