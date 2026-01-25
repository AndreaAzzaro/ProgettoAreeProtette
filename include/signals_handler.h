/**
 * @file signals_handler.h
 * @brief Gestione dei segnali per la sincronizzazione e terminazione dei processi.
 * 
 * Questo modulo isola la configurazione della `sigaction` e le flag atomiche
 * necessarie per la comunicazione asincrona via segnali UNIX.
 */

#ifndef SIGNALS_HANDLER_H
#define SIGNALS_HANDLER_H

#include <signal.h>
#include <stdbool.h>

/* ==========================================================================
 *                          VARIABILI GLOBALI (EXTERN)
 * ========================================================================= */

/** 
 * @brief Flag atomica per la gestione delle richieste di rifornimento (Innescata da SIGUSR2).
 */
extern volatile sig_atomic_t refill_request_flag;

/**
 * @brief Flag atomica per la gestione del blocco terminazione (Innescata da SIGINT/SIGTERM).
 */
extern volatile sig_atomic_t termination_requested_flag;

/* ==========================================================================
 *                          FUNZIONI PUBBLICHE
 * ========================================================================= */

/**
 * @brief Configura i gestori dei segnali per il processo Responsabile Mensa (Direttore).
 * 
 * Abilita la gestione di SIGINT per la chiusura controllata, SIGALRM per il timeout 
 * e SIGUSR1 per l'aggiunta dinamica di nuovi utenti alla simulazione.
 */
void configure_director_signal_handlers(void);

/**
 * @brief Configura i gestori dei segnali per i processi figli (Operatori, Utenti).
 * 
 * Permette alla popolazione della mensa di reagire a comandi esterni del Direttore
 * o a segnali di terminazione di emergenza.
 * 
 * @param simulation_running_status Puntatore alla flag di stato globale del processo figlio.
 * @param daily_cycle_running_status Puntatore alla flag del ciclo giornaliero attuale.
 */
void configure_child_signal_handlers(volatile sig_atomic_t *simulation_running_status, 
                                     volatile sig_atomic_t *daily_cycle_running_status);

#endif
