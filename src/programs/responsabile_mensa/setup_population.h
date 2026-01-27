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
void launch_simulation_users(MainSharedMemory *shared_memory_ptr);

/**
 * @brief Calcola il numero di gruppi necessari per la popolazione iniziale.
 * @param shm_ptr Puntatore alla memoria condivisa.
 * @return Numero di gruppi calcolati.
 */
int calculate_initial_groups_count(MainSharedMemory *shm_ptr);

/**
 * @brief Imposta la distribuzione degli operatori nelle varie stazioni.
 * @param shm_ptr Puntatore alla memoria condivisa.
 */
void setup_worker_distribution(MainSharedMemory *shm_ptr);

/**
 * @brief Inizializza i semafori dei posti operatore per tutte le stazioni.
 * 
 * Deve essere chiamata DOPO setup_worker_distribution() per avere i conteggi corretti.
 * 
 * @param shm_ptr Puntatore alla memoria condivisa.
 */
void initialize_station_operator_semaphores(MainSharedMemory *shm_ptr);

#endif
