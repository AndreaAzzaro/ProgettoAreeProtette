#ifndef OPERATORE_CASSA_H
#define OPERATORE_CASSA_H

#include "common.h"

/**
 * @struct StatoCassiere
 * @brief Rappresenta lo stato interno e le statistiche di un singolo operatore di cassa.
 */
typedef struct {
    int shmid;                          /* ID della risorsa SHM */
    int indice_postazione;              /* ID numerico della cassa */
    
    int bills_processed;               /* Statistica: pagamenti gestiti */
    int breaks_count;                  /* Statistica: numero di pause effettuate */

    SharedMemory *shm;                 /* Puntatore alla memoria condivisa agganciata */
} StatoCassiere;

/* Prototipi per la gestione del ciclo di vita del cassiere */
void init_cassiere(StatoCassiere *ca, int argc, char *argv[]);
void attendi_avvio_cassa(StatoCassiere *ca);
void ciclo_lavoro_cassa(StatoCassiere *ca);
void esegui_pausa_cassa(StatoCassiere *ca);

#endif
