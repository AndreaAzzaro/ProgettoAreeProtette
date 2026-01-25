/**
 * @file operatore_cassa.c
 * @brief Logica per gli operatori di cassa.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "operatore_cassa.h"
#include "signals_handler.h"

/* Variabili atomiche per il controllo del ciclo vitale */
volatile sig_atomic_t simulation_is_active = 1;
volatile sig_atomic_t daily_cycle_is_active = 1;

/**
 * @brief Punto di ingresso del processo Cassiere.
 */
int main(int argc, char *argv[]) {
    StatoCassiere cassiere;
    
    init_cassiere(&cassiere, argc, argv);

    /* Sincronizzazione di Startup */
    sync_child_start(cassiere.shm_ptr->semaphore_sync_id, BARRIER_STARTUP_READY, BARRIER_STARTUP_GATE);
    printf("[DEBUG] Cassiere PID %d ha superato la barriera di avvio.\n", getpid());

    /* Ciclo di vita persistente multi-giorno */
    while (simulation_is_active) {
        /* TODO: Logica giornaliera del cassiere */
    }
    return 0;
}

/**
 * @brief Inizializza lo stato del cassiere e lo collega alle risorse IPC.
 * 
 * @param ca Puntatore alla struttura di stato da inizializzare.
 * @param argc Conteggio argomenti linea di comando.
 * @param argv Vettore argomenti linea di comando.
 */
void init_cassiere(StatoCassiere *ca, int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[ERROR] Cassiere: shmid mancante.\n");
        exit(EXIT_FAILURE);
    }

    /* Reset struttura */
    for (size_t i = 0; i < sizeof(StatoCassiere); i++) ((char*)ca)[i] = 0;

    /* Parsing parametri e attach IPC */
    ca->shared_memory_id = atoi(argv[1]);
    ca->shm_ptr = attach_to_simulation_shared_memory(ca->shared_memory_id);

    /* Configurazione segnali asincroni */
    configure_child_signal_handlers(&simulation_is_active, &daily_cycle_is_active);

    printf("[DEBUG] Cassiere PID %d pronto.\n", getpid());
}
