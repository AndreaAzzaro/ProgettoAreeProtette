/**
 * @file add_users.c
 * @brief Utility esterna per aggiungere dinamicamente utenti alla simulazione.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "common.h"
#include "shm.h"
#include "sem.h"
#include "queue.h"
#include "utils.h"

/**
 * @brief Cerca uno slot libero nel pool di sincronizzazione gruppi.
 * @return Indice del gruppo libero o -1 se pieno.
 */
static int find_free_group_index(MainSharedMemory *shm) {
    for (int i = 0; i < shm->group_pool_size; i++) {
        if (shm->group_statuses[i].active_members == 0) {
            return i;
        }
    }
    return -1;
}

int main(int argc, char *argv[]) {
    /* Ora shmid non è più obbligatorio grazie a ftok() */
    if (argc > 3) {
        fprintf(stderr, "Uso: %s [numero_utenti]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* 1. Generazione chiave deterministica tramite ftok() */
    key_t key = ftok(IPC_KEY_PATH, IPC_PROJECT_ID);
    if (key == -1) {
        perror("[ERROR] ftok fallita. Assicurati che config/config.conf esista");
        return EXIT_FAILURE;
    }

    /* 2. Recupero shmid dal kernel */
    int shmid = shmget(key, 0, 0);
    if (shmid == -1) {
        fprintf(stderr, "[ERROR] Impossibile trovare la memoria condivisa della simulazione (shmget).\n");
        fprintf(stderr, "La simulazione è stata avviata?\n");
        return EXIT_FAILURE;
    }

    /* 3. Connessione alla memoria condivisa */
    MainSharedMemory *shm = attach_to_simulation_shared_memory(shmid);
    srand(time(NULL) ^ getpid());

    int users_to_add;
    if (argc >= 2) {
        users_to_add = atoi(argv[1]);
    } else {
        users_to_add = shm->configuration.quantities.number_of_new_users_batch;
        printf("[ADD_USERS] Nessun valore specificato. Uso default da config: %d\n", users_to_add);
    }

    if (users_to_add <= 0) {
        fprintf(stderr, "[ERROR] Numero utenti non valido (%d).\n", users_to_add);
        return EXIT_FAILURE;
    }

    /* 2. Invio richiesta al Master tramite coda di controllo */
    SimulationMessage msg;
    msg.message_type = MSG_TYPE_CONTROL;
    ControlPayload *payload = (ControlPayload *)msg.message_text;
    payload->users_count = users_to_add;

    if (send_message_to_queue(shm->control_queue_id, &msg, sizeof(ControlPayload), 0) == -1) {
        fprintf(stderr, "[ERROR] Invio richiesta alla coda di controllo fallito.\n");
        return EXIT_FAILURE;
    }

    /* 3. Notifica asincrona al Master */
    /* La flag add_users_flag viene impostata anche nell'handler del Master per sicurezza,
       ma qui garantiamo che sia 1 prima di mandare il segnale. */
    shm->add_users_flag = 1;
    if (kill(shm->master_pid, SIGUSR1) == -1) {
        perror("[ERROR] Segnale SIGUSR1 al Master fallito");
        return EXIT_FAILURE;
    }

    printf("[ADD_USERS] Richiesti %d utenti. Il Master li aggiungerà a fine giornata.\n", users_to_add);
    printf("[ADD_USERS] In attesa del permesso per lo spawn (sarà concesso alla chiusura del giorno)...\n");

    /* 4. Attesa autorizzazione (Atomic Handshake) 
       Il Master farà V() su questo semaforo solo dopo aver resettato le barriere per il nuovo giorno. */
    if (reserve_sem(shm->semaphore_mutex_id, MUTEX_ADD_USERS_PERMISSION) == -1) {
        perror("[ERROR] Attesa permesso fallita");
        return EXIT_FAILURE;
    }

    printf("[ADD_USERS] Autorizzazione ricevuta dal Master. Avvio spawn processi...\n");

    /* 5. Spawn dei nuovi utenti */
    int users_spawned = 0;
    while (users_spawned < users_to_add) {
        /* Calcola dimensione del prossimo gruppo */
        int group_size = (rand() % MAX_USERS_PER_GROUP) + 1;
        if (users_spawned + group_size > users_to_add) {
            group_size = users_to_add - users_spawned;
        }

        /* Trova slot nel pool gruppi */
        reserve_sem(shm->semaphore_mutex_id, MUTEX_SHARED_DATA);
        int sync_index = find_free_group_index(shm);
        if (sync_index == -1) {
            fprintf(stderr, "[ERROR] Pool gruppi saturo. Impossibile aggiungere altri utenti.\n");
            release_sem(shm->semaphore_mutex_id, MUTEX_SHARED_DATA);
            break;
        }
        
        /* Inizializza stato gruppo */
        shm->group_statuses[sync_index].active_members = group_size;
        shm->group_statuses[sync_index].group_leader_pid = 0;
        release_sem(shm->semaphore_mutex_id, MUTEX_SHARED_DATA);

        for (int i = 0; i < group_size; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                /* Figlio: Imposta PGID globale degli utenti per segnali broadcast */
                setpgid(0, shm->process_group_pids[GROUP_USERS]);

                char shm_str[24], gsize_str[24], gindex_str[24], is_leader_str[8];
                sprintf(shm_str, "%d", shmid);
                sprintf(gsize_str, "%d", group_size);
                sprintf(gindex_str, "%d", sync_index);
                sprintf(is_leader_str, "%d", (i == 0));

                execl("./bin/utente", "utente", shm_str, gsize_str, gindex_str, is_leader_str, (char *)NULL);
                perror("[ERROR] execl fallita");
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                /* Padre (Utility): Registrazione nel registro per gestione morti asincrone (SIGCHLD) */
                reserve_sem(shm->semaphore_mutex_id, MUTEX_SHARED_DATA);
                bool registered = false;
                for (int r = 0; r < MAX_USERS_REGISTRY; r++) {
                    if (shm->user_registry[r].pid == 0) {
                        shm->user_registry[r].pid = pid;
                        shm->user_registry[r].group_index = sync_index;
                        registered = true;
                        break;
                    }
                }
                release_sem(shm->semaphore_mutex_id, MUTEX_SHARED_DATA);
                
                if (!registered) {
                    fprintf(stderr, "[WARNING] Registro utenti pieno. PID %d non tracciato.\n", pid);
                }
            } else {
                perror("[ERROR] fork fallita");
            }
        }
        users_spawned += group_size;
    }

    printf("[ADD_USERS] Completato. %d utenti aggiunti correttamente alla simulazione.\n", users_spawned);
    return EXIT_SUCCESS;
}
