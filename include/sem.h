#ifndef SEM_H
#define SEM_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/**
 * @brief Unione necessaria per le operazioni semctl(SETVAL, etc.).
 * 
 * Definita secondo lo standard POSIX per garantire la compatibilità
 * del controllo dei semafori IPC System V.
 */
union semun {
    int val;                /**< Valore per SETVAL */
    struct semid_ds *buf;   /**< Buffer per IPC_STAT, IPC_SET */
    unsigned short *array;  /**< Array per GETALL, SETALL */
    struct seminfo *__buf;  /**< Buffer per IPC_INFO */
};

/**
 * @brief Crea un set di semafori.
 * 
 * @param key Chiave IPC generata (es. tramite ftok).
 * @param sem_num Numero di semafori da includere nel set.
 * @param flags Permessi e flag di creazione (es. IPC_CREAT, IPC_EXCL).
 * @return int ID del set di semafori in caso di successo, -1 in caso di errore.
 */
int create_sem_set(key_t key, int sem_num, int flags);

/**
 * @brief Inizializza il valore di un singolo semaforo nel set.
 * 
 * @param sem_id ID del set di semafori.
 * @param sem_num Indice del semaforo all'interno del set (0..nsems-1).
 * @param init_val Valore intero da assegnare al semaforo.
 * @return int 0 successo, -1 errore.
 */
int init_sem_val(int sem_id, int sem_num, int init_val);

/**
 * @brief Rimuove definitivamente un set di semafori dal sistema.
 * 
 * @param sem_id ID del set di semafori da eliminare.
 * @return int 0 successo, -1 errore.
 */
int delete_sem_set(int sem_id);

/**
 * @brief Esegue l'operazione di acquisizione risorsa (P).
 * 
 * Decrementa il valore del semaforo di 1. Se il valore è 0, il processo si blocca.
 * Utilizza SEM_UNDO. La funzione riprova automaticamente se interrotta da segnali.
 * 
 * @param sem_id ID del set di semafori.
 * @param semaphore_index Indice del semaforo su cui operare.
 * @return int 0 successo, -1 errore critico.
 */
int reserve_sem(int sem_id, int semaphore_index);

/**
 * @brief Esegue l'operazione di rilascio risorsa (V).
 * 
 * Incrementa il valore del semaforo di 1. Sblocca eventuali processi in attesa.
 * Utilizza SEM_UNDO.
 * 
 * @param sem_id ID del set di semafori.
 * @param semaphore_index Indice del semaforo su cui operare.
 * @return int 0 successo, -1 errore.
 */
int release_sem(int sem_id, int semaphore_index);

/**
 * @brief Incrementa il valore di un semaforo di una quantità specifica.
 * 
 * A differenza di release_sem, questa funzione NON utilizza SEM_UNDO.
 * Riprova automaticamente in caso di EINTR.
 * 
 * @param sem_id ID del set di semafori.
 * @param semaphore_index Indice del semaforo su cui operare.
 * @param increment_value Quantità da aggiungere al valore attuale.
 * @return int 0 successo, -1 errore.
 */
int release_sem_n(int sem_id, int semaphore_index, int increment_value);

/**
 * @brief Attende che il valore del semaforo diventi 0.
 * 
 * Riprova automaticamente se interrotta da segnali (EINTR).
 * 
 * @param sem_id ID del set di semafori.
 * @param semaphore_index Indice del semaforo su cui operare.
 * @return int 0 successo, -1 errore critico.
 */
int wait_for_zero(int sem_id, int semaphore_index);

/**
 * @brief Recupera il valore attuale di un semaforo.
 * 
 * @param sem_id ID del set di semafori.
 * @param sem_num Indice del semaforo all'interno del set.
 * @return int Il valore attuale (>=0) o -1 in caso di errore.
 */
int get_sem_val(int sem_id, int sem_num);

/**
 * @brief Inizializza o resetta una barriera di sincronizzazione.
 * 
 * Imposta il semaforo dei pronti al numero di processi attesi e chiude il cancello 
 * (impostandolo a 1).
 * 
 * @param sem_id ID del set di semafori.
 * @param ready_idx Indice del semaforo dei pronti.
 * @param gate_idx Indice del semaforo cancello.
 * @param n_processes Numero di processi da attendere.
 * @return int 0 successo, -1 errore.
 */
int setup_barrier(int sem_id, int ready_idx, int gate_idx, int n_processes);

/**
 * @brief Apre il cancello della barriera, sbloccando i processi in attesa.
 * 
 * Imposta il valore del semaforo cancello a 0.
 * 
 * @param sem_id ID del set di semafori.
 * @param gate_idx Indice del semaforo cancello.
 * @return int 0 successo, -1 errore.
 */
int open_barrier_gate(int sem_id, int gate_idx);

/**
 * @brief Sincronizza l'avvio di un processo figlio.
 * 
 * Utilizza una barriera a due stadi: incrementa (idealmente decrementa per segnalare presenza) 
 * un semaforo di pronti e attende che il cancello (gate) diventi zero.
 * 
 * @param sem_id ID del set di semafori di avvio.
 * @param ready_idx Indice del semaforo su cui segnalare la disponibilità.
 * @param gate_idx Indice del semaforo cancello da attendere.
 * @return int 0 successo, -1 errore.
 */
int sync_child_start(int sem_id, int ready_idx, int gate_idx);

#endif
