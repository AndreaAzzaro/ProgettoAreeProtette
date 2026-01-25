/**
 * @file signals_handler.c
 * @brief Implementazione dei gestori di segnale.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "signals_handler.h"

/* Definizione variabili globali atomiche */
volatile sig_atomic_t refill_request_flag = 0;
volatile sig_atomic_t termination_requested_flag = 0;

/* Puntatori a flag locali del processo figlio */
static volatile sig_atomic_t *p_sim_running = NULL;
static volatile sig_atomic_t *p_day_running = NULL;

/**
 * @brief Handler universale per terminazione pulita.
 */
static void handle_termination(int sig) {
    termination_requested_flag = 1;
    if (p_sim_running) *p_sim_running = 0;
    if (p_day_running) *p_day_running = 0;
}

/**
 * @brief Handler per eventi specifici della simulazione.
 */
static void handle_simulation_event(int sig) {
    if (sig == SIGUSR1) {
        /* Es: richiesta dinamica nuovi utenti */
    } else if (sig == SIGUSR2) {
        /* Fine giornata per i figli o richiesta refill */
        if (p_day_running) *p_day_running = 0;
        refill_request_flag = 1;
    }
}

void configure_director_signal_handlers(void) {
    struct sigaction sa_term, sa_event;

    /* Configurazione terminazione */
    sa_term.sa_handler = handle_termination;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;
    sigaction(SIGINT, &sa_term, NULL);
    sigaction(SIGTERM, &sa_term, NULL);

    /* Configurazione eventi */
    sa_event.sa_handler = handle_simulation_event;
    sigemptyset(&sa_event.sa_mask);
    sa_event.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa_event, NULL);
    sigaction(SIGUSR2, &sa_event, NULL);
}

void configure_child_signal_handlers(volatile sig_atomic_t *simulation_running_status, 
                                     volatile sig_atomic_t *daily_cycle_running_status) {
    p_sim_running = simulation_running_status;
    p_day_running = daily_cycle_running_status;

    struct sigaction sa;
    sa.sa_handler = handle_termination;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    struct sigaction sa_cycle;
    sa_cycle.sa_handler = handle_simulation_event;
    sigemptyset(&sa_cycle.sa_mask);
    sa_cycle.sa_flags = SA_RESTART;
    sigaction(SIGUSR2, &sa_cycle, NULL);
}
