/**
 * @file utente.h
 * @brief Definizioni per il processo Utente.
 * 
 * Gestisce lo stato, le scelte del menu e la sincronizzazione del processo
 * che usufruisce dei servizi della mensa.
 */

#ifndef UTENTE_H
#define UTENTE_H

#include "common.h"
#include <stdbool.h>

/**
 * @struct StatoUtente
 * @brief Rappresenta lo stato interno e le proprietà di un utente della mensa.
 */
typedef struct {
    int shared_memory_id;               /**< ID della risorsa SHM passata via linea di comando */
    bool has_ticket;                    /**< Possesso del ticket sconto (Versione Completa) */
    int group_id;                       /**< ID del gruppo di appartenenza (per mangiare insieme) */
    int group_size;                     /**< Dimensione totale del proprio gruppo */
    bool is_group_leader;               /**< Se l'utente è incaricato di coordinare il gruppo */

    MainSharedMemory *shm_ptr;          /**< Puntatore alla memoria condivisa agganciata */
    
    /* Scelte Menu del giorno (Indici negli array del Menu in SHM, -1 se non scelto) */
    int selected_first_course_index;
    int selected_second_course_index;
    int selected_dessert_coffee_index;

    int group_patience_threshold;       /**< Tempo massimo di attesa prima dell'abbandono (condiviso nel gruppo) */
} StatoUtente;

/**
 * @brief Inizializza la struttura stato utente analizzando gli argomenti d'ingresso.
 * 
 * @param utente Puntatore alla struttura da inizializzare.
 * @param argc Numero di argomenti da main.
 * @param argv Vettore degli argomenti da main.
 */
void init_utente(StatoUtente *utente, int argc, char *argv[]);

/**
 * @brief Sincronizza l'utente con il Direttore sulla barriera mattutina.
 * 
 * @param utente Puntatore allo stato utente.
 */
void attendi_avvio_utente(StatoUtente *utente);

/**
 * @brief Esegue l'intero percorso dell'utente (coda, scelta, pagamento, pasto).
 * 
 * @param utente Puntatore allo stato utente.
 */
void esegui_percorso_mensa(StatoUtente *utente);

#endif
