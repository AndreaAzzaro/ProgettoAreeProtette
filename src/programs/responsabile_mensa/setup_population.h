#ifndef SETUP_POPULATION_H
#define SETUP_POPULATION_H

/** Forward declaration: evita di includere common.h nell'header */
struct MainSharedMemory;

/**
 * @brief Calcola matematicamente la distribuzione dei lavoratori nelle stazioni.
 * 
 * @param total_workers Numero totale di lavoratori da distribuire.
 * @param average_times_array Array dei tempi medi di servizio per stazione.
 * @param distribution_results_array Array in cui salvare il numero di operatori per stazione.
 * @param stations_count Numero di stazioni totali.
 */
void calculate_worker_distribution(int total_workers, int *average_times_array, int *distribution_results_array, int stations_count);

/**
 * @brief Esegue il lancio (fork ed exec) di tutti i processi operatore.
 * 
 * @param shared_memory_ptr Puntatore alla struttura di memoria condivisa.
 */
void launch_simulation_operators(struct MainSharedMemory *shared_memory_ptr);

/**
 * @brief Esegue lo spawn di un blocco di processi utente.
 * 
 * @param shared_memory_ptr Puntatore alla memoria condivisa.
 * @param users_to_launch_count Numero di utenti da creare in questa chiamata.
 */
void launch_simulation_users(struct MainSharedMemory *shared_memory_ptr, int users_to_launch_count);

#endif
