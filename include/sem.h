#ifndef SEM_H
#define SEM_H

#include <sys/types.h>
#include <sys/sem.h>

/* Wrapper per i semafori System-V */

/* 
 * Union semun: necessaria per semctl().
 * Va definita se _SEM_SEMUN_UNDEFINED è presente (non standard su tutti i sistemi).
 */
#if defined(__linux__) && defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun già definita */
#else
union semun {
    int val;                /* Valore per SETVAL */
    struct semid_ds *buf;   /* Buffer per IPC_STAT, IPC_SET */
    unsigned short *array;  /* Array per GETALL, SETALL */
    struct seminfo *__buf;  /* Buffer per IPC_INFO (Linux specific) */
};
#endif

/*
 * Crea o ottiene un set di semafori (wrapper di semget).
 * Returns: id del semaforo o -1 on error.
 */
int create_sem_set(key_t key, int nsems, int flags);

/*
 * Inizializza il valore di un semaforo specifico (wrapper di semctl SETVAL).
 */
int init_sem_val(int sem_id, int sem_num, int init_val);

/*
 * Rimuove l'intero set di semafori (wrapper di semctl IPC_RMID).
 */
int delete_sem_set(int sem_id);

/*
 * Operazione P (Wait/Reserve): decrementa sops.sem_op = -1.
 * Blocca se il semaforo è 0.
 * Deve gestire EINTR.
 */
int reserve_sem(int sem_id, int sem_num);

/*
 * Operazione V (Signal/Release): incrementa sops.sem_op = +1.
 * Sblocca i processi in attesa.
 */
int release_sem(int sem_id, int sem_num);

/*
 * Operazione Z (Wait for Zero): sops.sem_op = 0.
 * Blocca finché il semaforo non diventa 0.
 * Utile per barriere o sincronizzazione all'avvio.
 */
int wait_for_zero(int sem_id, int sem_num);


#endif
