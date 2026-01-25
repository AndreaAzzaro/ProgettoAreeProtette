#ifndef UTENTE_H
#define UTENTE_H

#include "common.h"
#include <stdbool.h>

/**
 * @struct StatoUtente
 * @brief Rappresenta lo stato interno e le proprietà di un utente della mensa.
 */
typedef struct {
    int shmid;                          /* ID della risorsa SHM */
    bool has_ticket;                    /* Possesso del ticket sconto */
    int group_id;                       /* ID del gruppo di appartenenza */
    int group_size;                     /* Dimensione totale del proprio gruppo */
    bool is_leader;                     /* Se l'utente è il coordinatore del gruppo */

    SharedMemory *shm;                  /* Puntatore alla memoria condivisa agganciata */
    
    // Scelte Menu del giorno (Indici negli array del Menu in SHM, -1 se non scelto)
    int selected_first_idx;
    int selected_second_idx;
    int selected_bar_idx;

    int group_patience_threshold;       /* Soglia di sopportazione coda (condivisa nel gruppo) */
} StatoUtente;

/* Prototipi per la gestione del ciclo di vita dell'utente */
void init_utente(StatoUtente *utente, int argc, char *argv[]);
void attendi_avvio_utente(StatoUtente *utente);
void esegui_percorso_mensa(StatoUtente *utente);

#endif
