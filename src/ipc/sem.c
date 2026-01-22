#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "sem.h"

int create_sem_set(key_t key, int nsems, int flags) {
    int sem_id;
    if ((sem_id = semget(key, nsems, flags)) == -1) {
        perror("create_sem_set: semget failed");
    }
    return sem_id;
}

int init_sem_val(int sem_id, int sem_num, int init_val) {
    union semun arg;
    arg.val = init_val;
    
    if (semctl(sem_id, sem_num, SETVAL, arg) == -1) {
        perror("init_sem_val: semctl failed");
        return -1;
    }
    return 0;
}

int delete_sem_set(int sem_id) {
    /* 
     * IPC_RMID rimuove l'intero set di semafori.
     * Il secondo argomento è ignorato per IPC_RMID, ma passiamo 0 per sicurezza.
     */
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("delete_sem_set: semctl failed");
        return -1;
    }
    return 0;
}

/*
 * Funzione helper interna per eseguire operazioni semaforiche.
 * Gestisce il retry in caso di EINTR (interruzione da segnale).
 */
static int handle_sem_op(int sem_id, int sem_num, short op) {
    struct sembuf sops;
    sops.sem_num = sem_num;
    sops.sem_op = op;
    sops.sem_flg = 0; /* Nota: usando 0, se il processo crasha il semaforo non viene ripristinato automaticamente (no SEM_UNDO) */

    while (semop(sem_id, &sops, 1) == -1) {
        if (errno == EINTR) {
            /* Interrotto da un segnale, riprova l'operazione */
            continue;
        }
        /* Errore reale */
        perror("semop failed");
        return -1;
    }
    return 0;
}

int reserve_sem(int sem_id, int sem_num) {
    return handle_sem_op(sem_id, sem_num, -1);
}

int release_sem(int sem_id, int sem_num) {
    return handle_sem_op(sem_id, sem_num, 1);
}

int wait_for_zero(int sem_id, int sem_num) {
    return handle_sem_op(sem_id, sem_num, 0);
}
