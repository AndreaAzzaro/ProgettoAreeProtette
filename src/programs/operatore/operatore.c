/**
 * @file operatore.c
 * @brief Logica per gli operatori delle stazioni di distribuzione cibo.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "operatore.h"
#include "signals_handler.h"

/* Variabili atomiche per il controllo del ciclo vitale */
volatile sig_atomic_t simulation_is_active = 1;
volatile sig_atomic_t daily_cycle_is_active = 1;

/**
 * @brief Punto di ingresso del processo Operatore.
 */
int main(int argc, char *argv[]) {
    StatoOperatore operatore;
    
    init_operatore(&operatore, argc, argv);

    /* Sincronizzazione di Startup */
    sync_child_start(operatore.shm_ptr->semaphore_sync_id, BARRIER_STARTUP_READY, BARRIER_STARTUP_GATE);
    printf("[DEBUG] Operatore PID %d ha superato la barriera di avvio.\n", getpid());

    /* Ciclo di vita persistente multi-giorno */
    while (simulation_is_active) {
        /* TODO: Logica giornaliera dell'operatore */
    }
    return 0;
}

/**
 * @brief Inizializza lo stato dell'operatore e lo collega alle risorse IPC.
 * 
 * @param op Puntatore alla struttura di stato da inizializzare.
 * @param argc Conteggio argomenti linea di comando.
 * @param argv Vettore argomenti linea di comando.
 */
void init_operatore(StatoOperatore *op, int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "[ERROR] Operatore: Argomenti insufficienti.\n");
        exit(EXIT_FAILURE);
    }

    /* Reset struttura */
    for (int i = 0; i < sizeof(StatoOperatore); i++) ((char*)op)[i] = 0;

    /* Parsing parametri e attach IPC */
    op->shared_memory_id = atoi(argv[1]);
    op->station_type = atoi(argv[2]);
    op->shm_ptr = attach_to_simulation_shared_memory(op->shared_memory_id);

    /* Configurazione segnali asincroni */
    configure_child_signal_handlers(&simulation_is_active, &daily_cycle_is_active);

    printf("[DEBUG] Operatore PID %d pronto (Stazione %d).\n", getpid(), op->station_type);
}
