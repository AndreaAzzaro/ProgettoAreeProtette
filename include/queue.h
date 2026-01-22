#ifndef QUEUE_H
#define QUEUE_H

#include <sys/types.h>  
#include <sys/ipc.h>    
#include <sys/msg.h>    
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_MTEXT_SIZE 1024 // Dimensione arbitraria per il buffer di esempio

struct mymsg {
    long mtype;                 /* Message type (deve essere > 0) */
    char mtext[MAX_MTEXT_SIZE]; /* Message body */
};

/* * Crea o ottiene una coda di messaggi.
 * Utilizza msgget().
 * Parametri:
 * key: chiave generata con ftok() o IPC_PRIVATE[cite: 1185, 1186].
 * msgflg: permessi e flag (es. IPC_CREAT | IPC_EXCL)[cite: 1193, 1195].
 * Riferimento: Slide 28-30[cite: 1179, 1206].
 */
int create_queue(key_t key, int msgflg);

/* * Invia un messaggio alla coda.
 * Utilizza msgsnd().
 * Parametri:
 * msqid: identificatore della coda.
 * msgp: puntatore alla struct mymsg (cast a void*).
 * msgsz: dimensione del payload (mtext), non dell'intera struct.
 * msgflg: flag come IPC_NOWAIT[cite: 1286].
 * Riferimento: Slide 35-36[cite: 1271, 1272].
 */
int send_message(int msqid, struct mymsg *msgp, size_t msgsz, int msgflg);

/* * Riceve un messaggio dalla coda.
 * Utilizza msgrcv().
 * Parametri:
 * msqid: identificatore della coda.
 * msgp: puntatore al buffer dove salvare il messaggio.
 * maxmsgsz: dimensione massima accettabile del payload[cite: 1314].
 * msgtyp: selettore del tipo di messaggio (0, >0, <0) [cite: 1320-1322].
 * msgflg: flag come IPC_NOWAIT o MSG_NOERROR[cite: 1350, 1352].
 * Riferimento: Slide 37-41[cite: 1307, 1308].
 */
ssize_t receive_message(int msqid, struct mymsg *msgp, size_t maxmsgsz, long msgtyp, int msgflg);

/* * Rimuove la coda di messaggi.
 * Utilizza msgctl() con comando IPC_RMID.
 * Parametri:
 * msqid: identificatore della coda.
 * Riferimento: Slide 42-43[cite: 1377, 1378].
 */
int remove_queue(int msqid);

/* * Ottiene informazioni sulla coda (statistiche).
 * Utilizza msgctl() con comando IPC_STAT.
 * Parametri:
 * msqid: identificatore della coda.
 * buf: puntatore a struct msqid_ds dove salvare i dati.
 * Riferimento: Slide 44[cite: 1392].
 */
int get_queue_stats(int msqid, struct msqid_ds *buf);

#endif
