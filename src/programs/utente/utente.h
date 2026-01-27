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
    bool ticket_is_validated;           /**< Vero se il ticket è stato convalidato ieri/oggi */
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
 * @brief Esegue il ciclo di vita dell'utente per l'intera durata della simulazione.
 * 
 * @param utente Puntatore allo stato utente.
 */
void run_utente_simulation(StatoUtente *utente);

/* ==========================================================================
 *                          FASI OPERATIVE (LOGICA MODULARE)
 * ========================================================================= */

/** @brief Gestisce la coda e la validazione del ticket d'ingresso. */
void fase_validazione_ticket(StatoUtente *utente);

/** @brief Gestisce l'interazione con una stazione di cibo (Primi/Secondi). */
bool fase_servizio_stazione(StatoUtente *utente, int stazione_tipo);

/** @brief Gestisce il ritiro dalla mensa caricando i semafori di compensazione. */
void fase_ritiro_formale(StatoUtente *utente);

/** @brief Sincronizza i membri del gruppo al meeting point pre-cassa. */
void fase_riunione_gruppo(StatoUtente *utente);

/** @brief Simula il tempo di pagamento alle casse. */
void fase_pagamento_cassa(StatoUtente *utente, bool p1, bool p2);

/** @brief Gestisce la ricerca e prenotazione del tavolo (logica leader/amici). */
void fase_prenotazione_tavolo(StatoUtente *utente);

/** @brief Simula la consumazione del pasto e rilascia il posto a sedere. */
void fase_consumazione_pasto(StatoUtente *utente, bool p1, bool p2);

/** @brief Gestisce l'interazione con la stazione Bar/Dolci. */
void fase_servizio_caffe(StatoUtente *utente);

/** @brief Sincronizza il gruppo per l'uscita collettiva dalla mensa. */
void fase_uscita_collettiva(StatoUtente *utente);

/** @brief Aggiorna i contatori globali degli utenti serviti con successo. */
void aggiorna_statistiche_servito(StatoUtente *utente);

/** @brief Aggiorna i contatori globali degli utenti che hanno abbandonato. */
void aggiorna_statistiche_non_servito(StatoUtente *utente);

#endif
