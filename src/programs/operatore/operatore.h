#ifndef OPERATORE_H
#define OPERATORE_H

#include "common.h"

/**
 * @struct StatoOperatore
 * @brief Rappresenta lo stato interno e le statistiche di un singolo operatore di distribuzione.
 */
typedef struct {
    int shmid;                          /* ID della risorsa SHM */
    ProcessGroupIndex stazione;         /* Bancone di appartenenza (PRIMI, SECONDI, BAR) */
    int indice_postazione;              /* ID numerico all'interno del bancone */
    
    int portions_served;                /* Statistica: piatti serviti */
    int breaks_count;                   /* Statistica: numero di pause effettuate */

    SharedMemory *shm;                  /* Puntatore alla memoria condivisa agganciata */
} StatoOperatore;

/* Prototipi per la gestione del ciclo di vita dell'operatore */
void init_operatore(StatoOperatore *op, int argc, char *argv[]);
void attendi_avvio(StatoOperatore *op);
void ciclo_lavoro(StatoOperatore *op);
void esegui_pausa(StatoOperatore *op);

#endif
