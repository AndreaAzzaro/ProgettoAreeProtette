#ifndef SHM_H
#define SHM_H

#include <sys/types.h>

int create_shm(key_t key, size_t size);
int get_shm(key_t key, size_t size);
void *attach_shm(int shmid, int readonly);
int detach_shm(const void *shmaddr);
int remove_shm(int shmid);

#endif
