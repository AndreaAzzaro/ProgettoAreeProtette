/**
 * @file responsabile_mensa.c
 * @brief Entry point per il processo Master (Responsabile Mensa).
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "responsabile_mensa.h"

int main(int argc, char *argv[]) {
    printf("[MASTER] Responsabile Mensa in avvio...\n");

    /* 1. Caricamento Infrastruttura (L19-L30) */
    SimulationConfiguration configuration = load_simulation_configuration();
    SimulationMenu menu = load_simulation_menu();
    MainSharedMemory *shm_ptr = initialize_simulation_shared_memory();
    
    /* Configurazione Segnali (Compliance Consegna.md) */
    configure_director_signal_handlers();
    configure_child_signal_handlers(&shm_ptr->is_simulation_running, NULL);
    
    /* Copia configurazione e menu in SHM per accesso globale */
    shm_ptr->configuration = configuration;
    shm_ptr->food_menu = menu;

    /* 2. Inizializzazione Granulare Risorse IPC (L31-L38) */
    initialize_ipc_sources(shm_ptr);

    /* 3. Setup Popolazione (Spawn unico e persistente) */
    setup_worker_distribution(shm_ptr);

    /* Preparazione barriera di sincronizzazione (L41) */
    setup_prework_barrier(shm_ptr);

    /* Lancio processi figli */
    launch_simulation_operators(shm_ptr);
    launch_simulation_users(shm_ptr, shm_ptr->configuration.quantities.number_of_initial_users);

    /* Sincronizzazione Master sulla Barriera di Startup */
    synchronize_prework_barrier(shm_ptr);

    /* 4. Avvio Ciclo della Simulazione */
    start_simulation(shm_ptr);

    /* 5. Terminazione Coordinata dei Figli (Compliance Consegna.md) */
    printf("[MASTER] Fine simulazione rilevata. Notifica ai figli...\n");
    for (int i = 0; i < MAX_PROCESS_GROUPS; i++) {
        if (shm_ptr->process_group_pids[i] > 0) {
            /* Invia segnale a tutto il gruppo (segno negativo del PGID) */
            kill(-shm_ptr->process_group_pids[i], SIGTERM);
        }
    }

    /* 6. Attesa Terminazione e Pulizia */
    printf("[MASTER] In attesa della chiusura di tutti i processi...\n");
    while (wait(NULL) > 0);

    printf("[MASTER] Pulizia risorse IPC...\n");
    cleanup_ipc_resources(shm_ptr);

    return 0;
}

void initialize_ipc_sources(MainSharedMemory *shm_ptr){
    initialize_simulation_start_barriers(shm_ptr);
    initialize_daily_cycle_barriers(shm_ptr);
    initialize_global_simulation_mutexes(shm_ptr);
    initialize_food_distribution_station_semaphores(shm_ptr);
    initialize_dining_area_seats_semaphores(shm_ptr);
    initialize_ticket_validation_semaphores(shm_ptr);
    initialize_food_distribution_order_queues(shm_ptr);
    initialize_cashier_checkout_message_queue(shm_ptr);
}

void setup_worker_distribution(MainSharedMemory *shm_ptr) {
    int distribution[3];
    int average_times[3] = {
        shm_ptr->configuration.timings.average_service_time_primi,
        shm_ptr->configuration.timings.average_service_time_secondi,
        shm_ptr->configuration.timings.average_service_time_coffee
    };

    calculate_worker_distribution(
        shm_ptr->configuration.quantities.number_of_workers, 
        average_times, 
        distribution, 
        3
    );
    
    shm_ptr->first_course_station.num_operators_assigned = distribution[0];
    shm_ptr->second_course_station.num_operators_assigned = distribution[1];
    shm_ptr->coffee_dessert_station.num_operators_assigned = distribution[2];
}

void setup_prework_barrier(MainSharedMemory *shm_ptr) {
    int total_processes = 1 + 
                          shm_ptr->configuration.quantities.number_of_workers + 
                          shm_ptr->configuration.seats.seats_cash_desk + 
                          shm_ptr->configuration.quantities.number_of_initial_users;
    
    setup_barrier(shm_ptr->semaphore_sync_id, BARRIER_STARTUP_READY, BARRIER_STARTUP_GATE, total_processes);
}

void synchronize_prework_barrier(MainSharedMemory *shm_ptr) {
    printf("[MASTER] In attesa dei figli per il via libera globale...\n");
    
    /* 1. Il Master segnala la sua presenza */
    reserve_sem(shm_ptr->semaphore_sync_id, BARRIER_STARTUP_READY);
    
    /* 2. Attende che tutti i figli abbiano segnalato la presenza */
    wait_for_zero(shm_ptr->semaphore_sync_id, BARRIER_STARTUP_READY);
    
    /* 3. Apre il cancello per tutti (Decrementa GATE a 0) */
    reserve_sem(shm_ptr->semaphore_sync_id, BARRIER_STARTUP_GATE);
    
    printf("[MASTER] Barriera superata! Avvio simulazione.\n");
}

void start_simulation(MainSharedMemory *shm_ptr) {
    run_simulation_loop(shm_ptr);
}
