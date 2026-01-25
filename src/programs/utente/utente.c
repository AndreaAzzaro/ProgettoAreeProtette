/**
 * @file utente.c
 * @brief Entry point per il processo Utente.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "utente.h"
#include "signals_handler.h"

/* Variabili globali per la gestione dei segnali (Aderenza logic_blueprint.md) */
volatile sig_atomic_t simulation_is_active = 1;
volatile sig_atomic_t daily_cycle_is_active = 1;

int main(int argc, char *argv[]) {
    StatoUtente utente;
    
    init_utente(&utente, argc, argv);

    /* Sincronizzazione di Startup */
    sync_child_start(utente.shm_ptr->semaphore_sync_id, BARRIER_STARTUP_READY, BARRIER_STARTUP_GATE);
    printf("[DEBUG] Utente PID %d ha superato la barriera di avvio.\n", getpid());

    /* Ciclo di vita persistente multi-giorno */
    while (simulation_is_active) {
        /* TODO: Logica giornaliera dell'utente */
    }
    return 0;
}

/**
 * @brief Inizializza la struttura stato utente analizzando gli argomenti d'ingresso.
 * 
 * @param utente Puntatore alla struttura da inizializzare.
 * @param argc Numero di argomenti da main.
 * @param argv Vettore degli argomenti da main.
 */
void init_utente(StatoUtente *utente, int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[ERROR] Utente: shmid mancante.\n");
        exit(EXIT_FAILURE);
    }

    /* Reset struttura (Aderenza Function Shortness Policy) */
    for (size_t i = 0; i < sizeof(StatoUtente); i++) ((char*)utente)[i] = 0;

    /* Collegamento SHM */
    utente->shared_memory_id = atoi(argv[1]);
    utente->shm_ptr = attach_to_simulation_shared_memory(utente->shared_memory_id);

    /* Setup Segnali */
    configure_child_signal_handlers(&simulation_is_active, &daily_cycle_is_active);

    /* Inizializzazione Scelte Menu */
    utente->selected_first_course_index = -1;
    utente->selected_second_course_index = -1;
    utente->selected_dessert_coffee_index = -1;

    printf("[DEBUG] Utente PID %d inizializzato e collegato alla SHM %d.\n", getpid(), utente->shared_memory_id);
}
