/**
 * @file queue.c
 * @brief Implementazione delle astrazioni per le code di messaggi (System V IPC).
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "../../include/queue.h"

/* ==========================================================================
 *                          FUNZIONI PUBBLICHE
 * ========================================================================= */

int create_message_queue(key_t key, int message_flags) {
    int message_queue_id = msgget(key, message_flags);
    if (message_queue_id == -1) {
        perror("IPC Error: msgget failed in create_message_queue");
    }
    return message_queue_id;
}

int send_message_to_queue(int message_queue_id, SimulationMessage *message_pointer, size_t message_size, int message_flags) {
    int send_status = -1;
    bool operation_complete = false;

    /* Loop robusto per gestire EINTR senza usare 'continue' */
    while (operation_complete == false) {
        if (msgsnd(message_queue_id, (void *)message_pointer, message_size, message_flags) == -1) {
            if (errno != EINTR) {
                perror("IPC Error: msgsnd failed in send_message_to_queue");
                operation_complete = true; /* Errore critico: usciamo */
            }
            /* Se EINTR, il loop ricomincerà naturalmente */
        } else {
            send_status = 0;
            operation_complete = true;
        }
    }

    return send_status;
}

ssize_t receive_message_from_queue(int message_queue_id, SimulationMessage *message_pointer, size_t maximum_message_size, long message_type, int message_flags) {
    ssize_t bytes_received = -1;
    bool operation_complete = false;

    /* Loop robusto per gestire EINTR senza usare 'continue' */
    while (operation_complete == false) {
        ssize_t result = msgrcv(message_queue_id, (void *)message_pointer, maximum_message_size, message_type, message_flags);
        
        if (result == -1) {
            if (errno != EINTR) {
                /* Evitiamo il log di errore se la coda è vuota in modalità non bloccante */
                if (errno != ENOMSG) {
                    perror("IPC Error: msgrcv failed in receive_message_from_queue");
                }
                operation_complete = true;
            }
        } else {
            bytes_received = result;
            operation_complete = true;
        }
    }

    return bytes_received;
}

int remove_message_queue(int message_queue_id) {
    if (msgctl(message_queue_id, IPC_RMID, NULL) == -1) {
        perror("IPC Error: msgctl(IPC_RMID) failed");
        return -1;
    }
    return 0;
}

int get_message_queue_statistics(int message_queue_id, struct msqid_ds *statistics_buffer) {
    if (msgctl(message_queue_id, IPC_STAT, statistics_buffer) == -1) {
        perror("IPC Error: msgctl(IPC_STAT) failed");
        return -1;
    }
    return 0;
}
