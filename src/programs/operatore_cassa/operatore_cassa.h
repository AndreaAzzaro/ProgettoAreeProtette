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
 * @brief Configura gli handler per i segnali asincroni.
 */
void setup_cassiere_signals(void);

/**
 * @brief Esegue il ciclo di vita principale del cassiere (Settimana -> Giorno -> Lavoro).
 */
void run_cassiere_simulation(StatoCassiere *cassiere);

/* --- Funzioni di Fase (Dettagli del ciclo di vita) --- */

/**
 * @brief Implementa il ciclo di ricezione pagamenti (Loop 3).
 */
void fase_lavoro_cassa(StatoCassiere *cassiere);

/**
 * @brief Gestisce il rilascio della cassa e la decisione atomica della pausa.
 */
void fase_decisione_pausa_cassa(StatoCassiere *cassiere);

/**
 * @brief Simula il periodo di riposo del cassiere.
 */
void esegui_pausa_cassa(StatoCassiere *cassiere);

#endif
