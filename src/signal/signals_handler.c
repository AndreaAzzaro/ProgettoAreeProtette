/**
 * @file signals_handler.c
 * @brief Implementazione dei gestori di segnali per il sistema di simulazione.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "../../include/signals_handler.h"

/* ==========================================================================
 *                          COSTANTI E MACRO
 * ========================================================================= */

/** Prefisso per i messaggi di log relativi ai segnali */
#define SIGNAL_LOG_PREFIX "[SISTEMA-SEGNALI]"

/* ==========================================================================
 *                          VARIABILI GLOBALI
 * ========================================================================= */

volatile sig_atomic_t refill_request_flag = 0;
volatile sig_atomic_t termination_requested_flag = 0;

/* Puntatori interni per aggiornare lo stato nei processi figli */
static volatile sig_atomic_t *child_simulation_status_ptr = NULL;
static volatile sig_atomic_t *child_daily_cycle_status_ptr = NULL;

/* ==========================================================================
 *                          SIGNAL HANDLERS (PRIVATE)
 * ========================================================================= */

/**
 * @brief Gestore per i segnali di terminazione (SIGINT, SIGTERM).
 * 
 * Imposta la flag globale che verrà monitorata dal loop principale
 * per avviare la procedura di cleanup_ipc_resources.
 */
static void handle_termination_signal(int signal_number) {
    (void)signal_number;
    termination_requested_flag = 1;
    
    /* Se siamo un figlio, impostiamo a 0 la flag di esecuzione per uscire dai loop */
    if (child_simulation_status_ptr != NULL) {
        *child_simulation_status_ptr = 0;
    }
}

/**
 * @brief Gestore per la richiesta di rifornimento (SIGUSR2).
 */
static void handle_refill_request_signal(int signal_number) {
    (void)signal_number;
    refill_request_flag = 1;
}

/* ==========================================================================
 *                          FUNZIONI PUBBLICHE
 * ========================================================================= */

void configure_director_signal_handlers(void) {
    struct sigaction termination_action;
    struct sigaction refill_action;

    /* Configurazione per Terminazione (SIGINT, SIGTERM) */
    termination_action.sa_handler = handle_termination_signal;
    sigemptyset(&termination_action.sa_mask);
    termination_action.sa_flags = SA_RESTART; /* Evita fallimento system calls lente */

    sigaction(SIGINT, &termination_action, NULL);
    sigaction(SIGTERM, &termination_action, NULL);

    /* Configurazione per Refill (SIGUSR2) */
    refill_action.sa_handler = handle_refill_request_signal;
    sigemptyset(&refill_action.sa_mask);
    refill_action.sa_flags = SA_RESTART;

    sigaction(SIGUSR2, &refill_action, NULL);
}

void configure_child_signal_handlers(volatile sig_atomic_t *simulation_running_status, 
                                     volatile sig_atomic_t *daily_cycle_running_status) {
    
    /* Salviamo il riferimento alle flag del processo figlio per poterle resettare al volo */
    child_simulation_status_ptr = simulation_running_status;
    child_daily_cycle_status_ptr = daily_cycle_running_status;

    struct sigaction child_termination_action;
    
    child_termination_action.sa_handler = handle_termination_signal;
    sigemptyset(&child_termination_action.sa_mask);
    child_termination_action.sa_flags = SA_RESTART;

    sigaction(SIGINT, &child_termination_action, NULL);
    sigaction(SIGTERM, &child_termination_action, NULL);
}
