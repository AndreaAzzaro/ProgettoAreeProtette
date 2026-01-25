/**
 * @file operatore.h
 * @brief Definizioni per il processo Operatore di distribuzione cibo.
 */

#ifndef OPERATORE_H
#define OPERATORE_H

#include "common.h"

/**
 * @struct StatoOperatore
 * @brief Rappresenta lo stato interno e le statistiche di un singolo operatore di distribuzione.
 */
typedef struct {
    int shared_memory_id;               /**< ID della risorsa SHM passata via linea di comando */
    int station_type;                   /**< Bancone di appartenenza (0: PRIMI, 1: SECONDI, 2: BAR) */
    int assigned_post_index;            /**< ID numerico della postazione occupata all'interno del bancone */
    
    int total_portions_served;          /**< Statistica: piatti serviti nella giornata */
    int daily_breaks_taken;             /**< Statistica: numero di pause effettuate oggi */

    MainSharedMemory *shm_ptr;          /**< Puntatore alla memoria condivisa agganciata */
} StatoOperatore;

/**
 * @brief Inizializza la struttura stato operatore analizzando gli argomenti d'ingresso.
 * 
 * @param operatore Puntatore alla struttura da inizializzare.
 * @param argc Numero di argomenti da main.
 * @param argv Vettore degli argomenti da main.
 */
void init_operatore(StatoOperatore *operatore, int argc, char *argv[]);

/**
 * @brief Sincronizza l'operatore con il Direttore sulle barriere giornaliere.
 * 
 * @param operatore Puntatore allo stato operatore.
 */
void attendi_avvio_operatore(StatoOperatore *operatore);

/**
 * @brief Ciclo principale di servizio (attesa ordini e distribuzione).
 * 
 * @param operatore Puntatore allo stato operatore.
 */
void esegui_ciclo_lavoro(StatoOperatore *operatore);

/**
 * @brief Gestisce l'allontanamento temporaneo dalla postazione per una pausa.
 * 
 * @param operatore Puntatore allo stato operatore.
 */
void esegui_pausa_operatore(StatoOperatore *operatore);

#endif
