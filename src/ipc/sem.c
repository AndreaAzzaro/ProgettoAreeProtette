#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "sem.h"

/* ==========================================================================
 *                          FUNZIONI PRIVATE (STATIC)
 * ========================================================================= */

/**
 * @brief Esegue un'operazione atomica su un singolo semaforo.
 */
static int _execute_sem_op(int sem_id, int sem_idx, short op, short flags) {
    struct sembuf sb;
    sb.sem_num = (unsigned short)sem_idx;
    sb.sem_op = op;
    sb.sem_flg = flags;

    while (semop(sem_id, &sb, 1) == -1) {
        if (errno != EINTR) return -1;
    }
    return 0;
}

/* ==========================================================================
 *                          FUNZIONI PUBBLICHE
 * ========================================================================= */

int create_sem_set(key_t key, int sem_num, int flags) {
    int sem_id = semget(key, sem_num, flags);
    if (sem_id == -1) {
        perror("IPC Error: semget failed");
    }
    return sem_id;
}

int init_sem_val(int sem_id, int sem_num, int init_val) {
    union semun arg;
    arg.val = init_val;
    
    if (semctl(sem_id, sem_num, SETVAL, arg) == -1) {
        perror("IPC Error: semctl SETVAL failed");
        return -1;
    }
    return 0;
}

int delete_sem_set(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("IPC Error: semctl IPC_RMID failed");
        return -1;
    }
    return 0;
}

int reserve_sem(int sem_id, int semaphore_index) {
    return _execute_sem_op(sem_id, semaphore_index, -1, SEM_UNDO);
}

int release_sem(int sem_id, int semaphore_index) {
    return _execute_sem_op(sem_id, semaphore_index, 1, SEM_UNDO);
}

int release_sem_n(int sem_id, int semaphore_index, int increment_value) {
    return _execute_sem_op(sem_id, semaphore_index, (short)increment_value, 0);
}

int wait_for_zero(int sem_id, int semaphore_index) {
    return _execute_sem_op(sem_id, semaphore_index, 0, 0);
}

int get_sem_val(int sem_id, int sem_num) {
    int val = semctl(sem_id, sem_num, GETVAL);
    if (val == -1) {
        perror("IPC Error: semctl(GETVAL) failed");
    }
    return val;
}

int setup_barrier(int sem_id, int ready_idx, int gate_idx, int n_processes) {
    if (init_sem_val(sem_id, ready_idx, n_processes) == -1) return -1;
    return init_sem_val(sem_id, gate_idx, 1);
}

int open_barrier_gate(int sem_id, int gate_idx) {
    return init_sem_val(sem_id, gate_idx, 0);
}

int sync_child_start(int sem_id, int ready_idx, int gate_idx) {
    if (reserve_sem(sem_id, ready_idx) == -1) return -1;
    return wait_for_zero(sem_id, gate_idx);
}
