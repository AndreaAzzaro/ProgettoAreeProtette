/**
 * @file operatore_cassa.h
 * @brief Definizioni per il processo Operatore di Cassa (Cassiere).
 */

#ifndef OPERATORE_CASSA_H
#define OPERATORE_CASSA_H

#include "common.h"

/**
 * @struct StatoCassiere
 * @brief Rappresenta lo stato interno e le statistiche di un singolo operatore di cassa.
 */
typedef struct {
    int shared_memory_id;               /**< ID della risorsa SHM passata via linea di comando */
    int assigned_checkout_index;        /**< ID numerico della postazione cassa occupata */
    
    int total_customers_processed;      /**< Statistica: numero di pagamenti gestiti oggi */
    int daily_breaks_taken;             /**< Statistica: numero di pause effettuate oggi */

    MainSharedMemory *shm_ptr;          /**< Puntatore alla memoria condivisa agganciata */
} StatoCassiere;

/**
 * @brief Inizializza la struttura stato cassiere analizzando gli argomenti d'ingresso.
 * 
 * @param cassiere Puntatore alla struttura da inizializzare.
 * @param argc Numero di argomenti da main.
 * @param argv Vettore degli argomenti da main.
 */
void init_cassiere(StatoCassiere *cassiere, int argc, char *argv[]);

/**
 * @brief Sincronizza il cassiere con il Direttore sulle barriere giornaliere.
 * 
 * @param cassiere Puntatore allo stato cassiere.
 */
void attendi_avvio_cassa(StatoCassiere *cassiere);

/**
 * @brief Ciclo principale di gestione dei pagamenti alla cassa.
 * 
 * @param cassiere Puntatore allo stato cassiere.
 */
void esegui_ciclo_lavoro_cassa(StatoCassiere *cassiere);

/**
 * @brief Gestisce l'allontanamento temporaneo dalla cassa per una pausa.
 * 
 * @param cassiere Puntatore allo stato cassiere.
 */
void esegui_pausa_cassa(StatoCassiere *cassiere);

#endif
