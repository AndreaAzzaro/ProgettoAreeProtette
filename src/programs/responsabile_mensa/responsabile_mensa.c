/**
 * @file responsabile_mensa.c
 * @brief Punto di ingresso del Master (Responsabile Mensa).
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "common.h"
#include "responsabile_mensa.h"
#include "sem.h"

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    printf("[MASTER] Responsabile Mensa in avvio...\n");



    /* 2. Setup SHM e Risorse IPC con dimensione dinamica */
    /* Carichiamo i menu e la config localmente per calcolare le dimensioni SHM */
    SimulationConfiguration config = load_simulation_configuration();
    SimulationMenu menu = load_simulation_menu();
    
    /* Calcoliamo il pool dei gruppi basandoci sul numero di utenti iniziali + margine */
    int users_to_assign = config.quantities.number_of_initial_users;
    /* Caso peggiore: ogni utente in un gruppo differente + margine add_users */
    int dynamic_group_pool_size = users_to_assign + 100; 

    MainSharedMemory *shm_ptr = initialize_simulation_shared_memory(dynamic_group_pool_size);
    shm_ptr->configuration = config;
    shm_ptr->food_menu = menu;
    
    printf("[MASTER] SHMID: %d\n", shm_ptr->shared_memory_id);
    
    initialize_ipc_sources(shm_ptr);
    
    /* [SICUREZZA] Configurazione segnali fin dallo startup */
    setup_sigchld_handler(shm_ptr);
    setup_signal_close_day(shm_ptr);

    /* [DINAMICO] Calcolo gruppi e preparazione Pool Semafori */
    int total_required_groups = calculate_initial_groups_count(shm_ptr);
    initialize_group_sync_pool(shm_ptr, total_required_groups);

    /* 3. Setup Popolazione (Spawn unico e persistente) */
    setup_worker_distribution(shm_ptr);
    initialize_station_operator_semaphores(shm_ptr);

    /* Preparazione barriera di sincronizzazione (L41) */
    setup_prework_barrier(shm_ptr);

    /* [RISOLUZIONE INCOERENZA 2] Setup barriere operative (Morning/Evening) per il Giorno 1 */
    setup_daily_barriers(shm_ptr);
    
    /* [GRUPPI] Inizializzazione barriere di gruppo per il Giorno 1 */
    setup_group_barriers(shm_ptr);

    /* Lancio processi figli */
    launch_simulation_operators(shm_ptr);
    launch_simulation_users(shm_ptr);

    /* Sincronizzazione Master sulla Barriera di Startup */
    synchronize_prework_barrier(shm_ptr);

    /* 4. Avvio Ciclo della Simulazione */
    start_simulation(shm_ptr);

    /* 5. Terminazione Coordinata dei Figli (Compliance Consegna.md) */
    printf("[MASTER] Fine simulazione rilevata. Notifica ai figli...\n");
    terminate_simulation_gracefully(shm_ptr, EXIT_SUCCESS);

    return 0;
}

void setup_prework_barrier(MainSharedMemory *shm_ptr) {
    int total_processes = shm_ptr->configuration.quantities.number_of_workers + 
                        shm_ptr->configuration.seats.seats_cash_desk + 
                        shm_ptr->current_total_users;
    
    setup_barrier(shm_ptr->semaphore_sync_id, BARRIER_STARTUP_READY, BARRIER_STARTUP_GATE, total_processes);
}

void setup_daily_barriers(MainSharedMemory *shm_ptr) {
    int total_processes = shm_ptr->configuration.quantities.number_of_workers + 
                        shm_ptr->configuration.seats.seats_cash_desk + 
                        shm_ptr->current_total_users;
    
    /* Configura entrambe le barriere operative con il conteggio attuale della popolazione */
    setup_barrier(shm_ptr->semaphore_sync_id, BARRIER_MORNING_READY, BARRIER_MORNING_GATE, total_processes);
    setup_barrier(shm_ptr->semaphore_sync_id, BARRIER_EVENING_READY, BARRIER_EVENING_GATE, total_processes);
}

void synchronize_prework_barrier(MainSharedMemory *shm_ptr) {
    printf("[MASTER] In attesa dei figli per il via libera globale...\n");
    
    /* Attesa che il READY arrivi a 0 (Tutti i figli pronti) */
    wait_for_zero(shm_ptr->semaphore_sync_id, BARRIER_STARTUP_READY);
    
    /* Sblocco del cancello */
    open_barrier_gate(shm_ptr->semaphore_sync_id, BARRIER_STARTUP_GATE);
    printf("[MASTER] Barriera superata! Avvio simulazione.\n");
}

void start_simulation(MainSharedMemory *shm_ptr) {
    run_simulation_loop(shm_ptr);
}
