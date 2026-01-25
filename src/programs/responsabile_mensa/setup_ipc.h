#ifndef SETUP_TASKS_H
#define SETUP_TASKS_H

#include "common.h"

/* --- Inizializzazione Granulare Semafori --- */
void init_start_barrier(SharedMemory *shm);
void init_global_mutexes(SharedMemory *shm);
void init_station_semaphores(SharedMemory *shm);
void init_dining_area_semaphores(SharedMemory *shm);
void init_ticket_semaphores(SharedMemory *shm);

/* --- Inizializzazione Granulare Code di Messaggi --- */
void init_distribution_queues(SharedMemory *shm);
void init_checkout_queue(SharedMemory *shm);

#endif
