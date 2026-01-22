#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include "shm.h"

int create_shm(key_t key, size_t size) {
    int id;
    if ((id = shmget(key, size, IPC_CREAT | 0666))== -1) {
        perror("shmget create failed");
        return -1;
    }
    return id;
}

int get_shm(key_t key, size_t size) {
    int id;
    if ((id = shmget(key, size, 0666)) == -1) {
        perror("shmget get failed");
        return -1;
    }
    return id;
}

void *attach_shm(int shmid, int readonly) {
    int flags = 0;
    if (readonly)
        flags = SHM_RDONLY;

    void *ptr = shmat(shmid, NULL, flags);
    if (ptr == (void *)-1) {
        perror("shmat failed");
        return NULL;
    }
    return ptr;
}

int detach_shm(const void *shmaddr) {
    if (shmdt(shmaddr) == -1) {
        perror("shmdt failed");
        return -1;
    }
    return 0;
}

int remove_shm(int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID failed");
        return -1;
    }
    return 0;
}