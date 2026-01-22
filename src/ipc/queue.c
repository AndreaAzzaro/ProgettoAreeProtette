#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "queue.h"

int create_queue(key_t key, int msgflg) {
    int id = msgget(key, msgflg);
    if (id == -1) {
        perror("create_queue: msgget failed");
    }
    return id;
}

int send_message(int msqid, struct mymsg *msgp, size_t msgsz, int msgflg) {
    /* 
     * Ciclo per gestire l'interruzione da segnale (EINTR).
     * Nota: msgsnd si blocca solo se la coda è piena e non è specificato IPC_NOWAIT.
     */
    while (msgsnd(msqid, (void *)msgp, msgsz, msgflg) == -1) {
        if (errno == EINTR) {
            continue;
        }
        perror("send_message: msgsnd failed");
        return -1;
    }
    return 0;
}

ssize_t receive_message(int msqid, struct mymsg *msgp, size_t maxmsgsz, long msgtyp, int msgflg) {
    ssize_t bytes_read;
    
    /* 
     * Ciclo per gestire l'interruzione da segnale (EINTR).
     * msgrcv si blocca se non ci sono messaggi del tipo richiesto (e no IPC_NOWAIT).
     */
    while ((bytes_read = msgrcv(msqid, (void *)msgp, maxmsgsz, msgtyp, msgflg)) == -1) {
        if (errno == EINTR) {
            continue;
        }
        /* 
         * ENOMSG è un errore "atteso" se usiamo IPC_NOWAIT, 
         * quindi magari non vogliamo un perror chiassoso in quel caso specifico.
         * Qui lascio perror generico, ma tieni a mente se usi IPC_NOWAIT.
         */
        if (errno != ENOMSG) {
            perror("receive_message: msgrcv failed");
        }
        return -1;
    }
    return bytes_read;
}

int remove_queue(int msqid) {
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("remove_queue: msgctl IPC_RMID failed");
        return -1;
    }
    return 0;
}

int get_queue_stats(int msqid, struct msqid_ds *buf) {
    if (msgctl(msqid, IPC_STAT, buf) == -1) {
        perror("get_queue_stats: msgctl IPC_STAT failed");
        return -1;
    }
    return 0;
}
