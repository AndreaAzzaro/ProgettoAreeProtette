/**
 * @file utente.c
 * @brief Processo Utente per test barriera startup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include "utente.h"
#include "sem.h"
#include "shm.h"
#include "queue.h"
#include "utils.h"

static volatile sig_atomic_t local_daily_cycle_is_active = 0;

/* Prototipi Funzioni Private */
static void handle_utente_signals(int sig);
static void setup_utente_signals(void);
static void genera_identita_casuale(StatoUtente *utente);
static bool fase_checkout_piatto(StatoUtente *utente, FoodDistributionStation *stazione, int *choice, int stazione_tipo);
static ssize_t receive_message_with_soft_timeout(int queue_id, SimulationMessage *msg, size_t size, long type);

/**
 * @brief Calcola i minuti simulati trascorsi tra due timestamp.
 */
static double get_simulated_minutes(struct timespec start, struct timespec end, long nanosecs_per_tick) {
    long long delta_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    if (nanosecs_per_tick <= 0) return 0;
    return (double)delta_ns / nanosecs_per_tick;
}

/**
 * @brief Aggiorna l'accumulatore specifico in SHM per i tempi di attesa.
 */
static void update_wait_time_stat(StatoUtente *utente, double wait_min, int type) {
    reserve_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SIMULATION_STATS);
    WaitTimeAccumulator *acc = &utente->shm_ptr->statistics.daily_wait_accumulators;
    switch(type) {
        case 0: acc->sum_wait_first += wait_min; acc->count_first++; break;
        case 1: acc->sum_wait_second += wait_min; acc->count_second++; break;
        case 2: acc->sum_wait_coffee += wait_min; acc->count_coffee++; break;
        case 3: acc->sum_wait_cashier += wait_min; acc->count_cashier++; break;
    }
    release_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SIMULATION_STATS);
}

/* ==========================================================================
 *                          FUNZIONI PUBBLICHE
 * ========================================================================= */

int main(int argc, char *argv[]) {
    StatoUtente utente;
    
    if (argc < 5) {
        fprintf(stderr, "[ERROR] %s: Parametri insufficienti\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init_utente(&utente, argc, argv);
    /* Inizializzazione Random Seed con PID per diversificare le sequenze tra processi */
    srand(time(NULL) ^ getpid());

    /* Avvio del ciclo vita della simulazione */
    run_utente_simulation(&utente);

    printf("[UTENTE] PID %d: Terminazione pulita.\n", getpid());
    return 0;
}

void init_utente(StatoUtente *utente, int argc, char *argv[]) {
    (void)argc;
    
    /* Parsing Argomenti */
    utente->shared_memory_id = atoi(argv[1]);
    utente->group_size = atoi(argv[2]);
    int sync_index = atoi(argv[3]);
    utente->is_group_leader = (atoi(argv[4]) == 1);
    
    /* Group ID basato sull'indice nel pool per semplicità di log */
    utente->group_id = sync_index; 

    /* Attach SHM */
    utente->shm_ptr = attach_to_simulation_shared_memory(utente->shared_memory_id);

    /* Generazione Identità Random */
    genera_identita_casuale(utente);
}

void run_utente_simulation(StatoUtente *utente) {
    /* Setup Segnali */
    setup_utente_signals();

    /* Sincronizzazione di Startup Globale */
    /* Se la simulazione è già avviata (es. add_users durante il Giorno 1+), 
       saltiamo lo startup e andiamo direttamente al primo ciclo giornaliero disponibile. */
    if (utente->shm_ptr->current_simulation_day == 0) {
        printf("[DEBUG] Utente PID %d: In attesa barriera di Startup.\n", getpid());
        sync_child_start(utente->shm_ptr->semaphore_sync_id, BARRIER_STARTUP_READY, BARRIER_STARTUP_GATE);
    } else {
        printf("[DEBUG] Utente PID %d: Aggiunto a simulazione in corso (Giorno %d). Salto startup.\n", 
               getpid(), utente->shm_ptr->current_simulation_day);
    }
    
    /* Registrazione Leadership: il leader scrive il proprio PID in SHM */
    if (utente->is_group_leader) {
        int s_idx = utente->group_id;
        /* Mutex non strettamente necessario qui (startup atomica), ma usato per coerenza */
        reserve_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);
        utente->shm_ptr->group_statuses[s_idx].group_leader_pid = getpid();
        release_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);
    }

    printf("[DEBUG] Utente PID %d: Inizializzazione completata. Pronto.\n", getpid());

    /* Loop Permanente della Simulazione */
    while (utente->shm_ptr->is_simulation_running) {
        /* [ATO 3-4] Sincronizzazione Mattutina */
        local_daily_cycle_is_active = 1;
        
        /* [PUNTO 22] Reset Stato Day e Scelta Menu (Sez 5.5 Consegna: "Ogni giorno stabilisce il menu") */
        genera_identita_casuale(utente);
        utente->ticket_is_validated = false; 

        sync_child_start(utente->shm_ptr->semaphore_sync_id, BARRIER_MORNING_READY, BARRIER_MORNING_GATE);
        
        /* Logica operativa giornaliera */
        if (local_daily_cycle_is_active) {
            printf("[UTENTE] PID %d: Inizio giornata %d.\n", getpid(), utente->shm_ptr->current_simulation_day + 1);

            fase_validazione_ticket(utente);

            bool got_first = fase_servizio_stazione(utente, 0); /* 0: Primi */
            bool got_second = fase_servizio_stazione(utente, 1); /* 1: Secondi */

            /* Logica Abbandono: Solo se non ha preso nulla */
            if (local_daily_cycle_is_active && !got_first && !got_second) {
                fase_ritiro_formale(utente);
            }

            if (local_daily_cycle_is_active) {
                fase_riunione_gruppo(utente);
                fase_pagamento_cassa(utente, got_first, got_second);
                fase_prenotazione_tavolo(utente);
                fase_consumazione_pasto(utente, got_first, got_second);
                fase_servizio_caffe(utente);
                fase_uscita_collettiva(utente);
                aggiorna_statistiche_servito(utente);
            } else {
                /* Se la giornata è finita prematuramente o ha abbandonato */
                aggiorna_statistiche_non_servito(utente);
            }
        }

        /* [ATO 20-21] Sincronizzazione Serale */
        sync_child_start(utente->shm_ptr->semaphore_sync_id, BARRIER_EVENING_READY, BARRIER_EVENING_GATE);
    }
}

/* ==========================================================================
 *                          FUNZIONI PRIVATE (STATIC)
 * ========================================================================= */

/**
 * @brief Handler per segnali Master (Fine Giorno / Fine Simulazione).
 */
static void handle_utente_signals(int sig) {
    if (sig == SIGUSR2 || sig == SIGTERM || sig == SIGINT) {
        local_daily_cycle_is_active = 0;
    }
}

/**
 * @brief Configura gli handler per i segnali asincroni.
 */
static void setup_utente_signals(void) {
    struct sigaction sa;
    sa.sa_handler = handle_utente_signals;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
}

void fase_validazione_ticket(StatoUtente *utente) {
    if (utente->has_ticket && local_daily_cycle_is_active) {
        printf("[UTENTE] PID %d: In coda per validazione ticket...\n", getpid());
        if (reserve_sem(utente->shm_ptr->semaphore_ticket_id, 0) != -1) {
            /* Check post-attesa semaforo */
            if (local_daily_cycle_is_active) {
                usleep(utente->shm_ptr->configuration.timings.average_service_time_ticket);
                utente->ticket_is_validated = true;
                printf("[UTENTE] PID %d: Ticket validato.\n", getpid());
            }
            release_sem(utente->shm_ptr->semaphore_ticket_id, 0);
        }
    }
}

bool fase_servizio_stazione(StatoUtente *utente, int stazione_tipo) {
    if (!local_daily_cycle_is_active) return false;

    /* 1. Selezione Riferimenti Stazione */
    FoodDistributionStation *stazione = (stazione_tipo == 0) ? 
                &utente->shm_ptr->first_course_station : 
                &utente->shm_ptr->second_course_station;
    
    int choice = (stazione_tipo == 0) ? 
                utente->selected_first_course_index : 
                utente->selected_second_course_index;

    if (choice == -1) return false;

    /* 2. Check Disponibilità e Strategia di Ripiego (Atomico con Mutex) */
    reserve_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);
    
    if (stazione->portions[choice] <= 0) {
        int num_dishes = (stazione_tipo == 0) ? 
                        utente->shm_ptr->food_menu.number_of_first_courses : 
                        utente->shm_ptr->food_menu.number_of_second_courses;
        
        bool found_alternative = false;
        for (int i = 0; i < num_dishes; i++) {
            if (stazione->portions[i] > 0) {
                choice = i;
                found_alternative = true;
                break;
            }
        }

        if (!found_alternative) {
            release_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);
            printf("[UTENTE] PID %d: Tutti i piatti terminati alla stazione %s.\n", 
                   getpid(), (stazione_tipo == 0 ? "Primi" : "Secondi"));
            return false;
        }
        printf("[UTENTE] PID %d: Piatto preferito terminato. Scelgo l'alternativa index %d.\n", getpid(), choice);
    }
    
    release_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);

    /* 3. Check Coda (Soglia Pazienza) */
    int q_len = get_message_queue_length(stazione->message_queue_id);
    if (q_len > utente->shm_ptr->configuration.thresholds.queue_patience_threshold) {
        printf("[UTENTE] PID %d: Troppa coda alla stazione %s (%d utenti). Passo oltre.\n", 
               getpid(), (stazione_tipo == 0 ? "Primi" : "Secondi"), q_len);
        return false;
    }

    /* 4. Interazione IPC (Punto 4 Refactoring: fase_checkout_piatto) */
    return fase_checkout_piatto(utente, stazione, &choice, stazione_tipo);
}

static void genera_identita_casuale(StatoUtente *utente) {
    /* 1. Ticket: Ratio 4:1 garantito deterministicamente usando il PID.
       (PID % 5) != 0 restituisce TRUE per 4 valori su 5 (1,2,3,4) e FALSE per 1 (0). */
    utente->has_ticket = ((getpid() % 5) != 0);

    /* 2. Scelte Menu */
    utente->selected_first_course_index = generate_random_integer(0, MAX_DISHES_PER_CATEGORY - 1);
    utente->selected_second_course_index = generate_random_integer(0, MAX_DISHES_PER_CATEGORY - 1);
    utente->selected_dessert_coffee_index = generate_random_integer(0, 3);

    /* 3. Pazienza (es. tra 30 e 120 minuti simulati) */
    utente->group_patience_threshold = generate_random_integer(30, 120);
}

/**
 * @brief Riceve un messaggio con timeout soft (non bloccante + retry).
 * 
 * Evita il blocco permanente se l'operatore muore. Usa IPC_NOWAIT con
 * un breve sleep tra i tentativi per non saturare la CPU.
 * 
 * @return ssize_t Bytes ricevuti, o -1 se errore/fine giornata.
 */
static ssize_t receive_message_with_soft_timeout(int queue_id, SimulationMessage *msg, size_t size, long type) {
    while (local_daily_cycle_is_active) {
        ssize_t result = receive_message_from_queue(queue_id, msg, size, type, IPC_NOWAIT);
        if (result != -1) {
            return result; /* Messaggio ricevuto con successo */
        }
        if (errno == ENOMSG) {
            usleep(10000); /* 10ms - breve pausa per non saturare CPU */
        } else {
            return -1; /* Errore critico (non ENOMSG) */
        }
    }
    return -1; /* Fine giornata durante l'attesa */
}

static bool fase_checkout_piatto(StatoUtente *utente, FoodDistributionStation *stazione, int *choice, int stazione_tipo) {
    struct timespec start_t, end_t;
    clock_gettime(CLOCK_MONOTONIC, &start_t);
    
    SimulationMessage msg;
    msg.message_type = MSG_TYPE_ORDER;
    
    StationPayload *payload = (StationPayload *)msg.message_text;
    payload->user_pid = getpid();
    payload->dish_index = *choice;
    payload->status = 0;

    printf("[UTENTE] PID %d: In coda alla stazione %s per il piatto %d.\n", 
           getpid(), (stazione_tipo == 0 ? "Primi" : (stazione_tipo == 1 ? "Secondi" : "Caffè")), *choice);

    if (send_message_to_queue(stazione->message_queue_id, &msg, sizeof(StationPayload), 0) == -1) {
        return false;
    }

    /* 5. Attesa Risposta con timeout soft (Filtro sul proprio PID) */
    if (receive_message_with_soft_timeout(stazione->message_queue_id, &msg, sizeof(StationPayload), getpid()) == -1) {
        return false;
    }

    /* Check post-attesa coda: se la giornata è finita, il piatto non conta */
    if (!local_daily_cycle_is_active) return false;

    clock_gettime(CLOCK_MONOTONIC, &end_t);
    double wait_min = get_simulated_minutes(start_t, end_t, utente->shm_ptr->configuration.timings.nanoseconds_per_tick);
    update_wait_time_stat(utente, wait_min, stazione_tipo);

    if (payload->status == ORDER_STATUS_SERVED) {
        printf("[UTENTE] PID %d: Ricevuto piatto index %d dalla stazione.\n", getpid(), *choice);
        return true;
    } else {
        printf("[UTENTE] PID %d: Piatto %d non ottenuto (esaurito durante l'attesa).\n", getpid(), *choice);
        return false; 
    }
}

void fase_ritiro_formale(StatoUtente *utente) {
    printf("[UTENTE] PID %d: Nessun piatto ottenuto. Abbandono anticipato.\n", getpid());
    local_daily_cycle_is_active = 0;
    
    int s_idx = utente->group_id;
    reserve_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);
    if (utente->shm_ptr->group_statuses[s_idx].active_members > 0) {
        utente->shm_ptr->group_statuses[s_idx].active_members--;
        if (utente->is_group_leader) {
            utente->shm_ptr->group_statuses[s_idx].group_leader_pid = 0;
            utente->is_group_leader = false;
        }
    }
    release_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);

    int base_sem = s_idx * GROUP_SEMS_PER_ENTRY;
    reserve_sem_no_undo(utente->shm_ptr->group_sync_semaphore_id, base_sem + GROUP_SEM_PRE_CASHIER);
    reserve_sem_no_undo(utente->shm_ptr->group_sync_semaphore_id, base_sem + GROUP_SEM_EXIT);
}

void fase_riunione_gruppo(StatoUtente *utente) {
    if (utente->group_size <= 1 || !local_daily_cycle_is_active) return;

    int s_idx = utente->group_id;
    reserve_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);
    if (utente->shm_ptr->group_statuses[s_idx].group_leader_pid == 0) {
        utente->shm_ptr->group_statuses[s_idx].group_leader_pid = getpid();
        utente->is_group_leader = true;
        printf("[UTENTE] PID %d: Assumo leadership per riunione gruppo %d.\n", getpid(), s_idx);
    }
    release_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);

    int base_sem = s_idx * GROUP_SEMS_PER_ENTRY;
    printf("[UTENTE] PID %d: Attesa amici al meeting point...\n", getpid());
    
    /* Usiamo NO_UNDO per evitare deadlock se un membro muore dopo la reserve */
    if (reserve_sem_no_undo(utente->shm_ptr->group_sync_semaphore_id, base_sem + GROUP_SEM_PRE_CASHIER) != -1) {
        if (local_daily_cycle_is_active) {
            wait_for_zero(utente->shm_ptr->group_sync_semaphore_id, base_sem + GROUP_SEM_PRE_CASHIER);
        }
    }
}

void fase_pagamento_cassa(StatoUtente *utente, bool p1, bool p2) {
    if (!local_daily_cycle_is_active) return;

    struct timespec start_t, end_t;
    clock_gettime(CLOCK_MONOTONIC, &start_t);

    SimulationMessage msg;
    msg.message_type = MSG_TYPE_ORDER; 
    
    CashierPayload *payload = (CashierPayload *)msg.message_text;
    payload->user_pid = getpid();
    payload->had_first = p1;
    payload->had_second = p2;
    payload->want_coffee = true; /* Per ora sempre vero se arriva in cassa */
    payload->has_discount = utente->ticket_is_validated;

    printf("[UTENTE] PID %d: In coda alla Cassa (Piatti presi: %d)...\n", 
           getpid(), (p1?1:0) + (p2?1:0));

    if (send_message_to_queue(utente->shm_ptr->register_station.message_queue_id, &msg, sizeof(CashierPayload), 0) == -1) {
        return;
    }

    /* Attesa feedback dal cassiere con timeout soft */
    if (receive_message_with_soft_timeout(utente->shm_ptr->register_station.message_queue_id, &msg, sizeof(CashierPayload), getpid()) == -1) {
        return;
    }
    
    if (!local_daily_cycle_is_active) return;

    clock_gettime(CLOCK_MONOTONIC, &end_t);
    double wait_min = get_simulated_minutes(start_t, end_t, utente->shm_ptr->configuration.timings.nanoseconds_per_tick);
    update_wait_time_stat(utente, wait_min, 3); /* 3: Cassa */

    printf("[UTENTE] PID %d: Pagamento completato. Ricevuto scontrino dal Cassiere.\n", getpid());
}

void fase_prenotazione_tavolo(StatoUtente *utente) {
    if (!local_daily_cycle_is_active) return;

    int s_idx = utente->group_id;
    int base_sem = s_idx * GROUP_SEMS_PER_ENTRY;

    if (utente->is_group_leader) {
        reserve_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);
        int members = utente->shm_ptr->group_statuses[s_idx].active_members;
        release_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SHARED_DATA);

        printf("[UTENTE] PID %d (Leader): Prenoto tavolo per %d.\n", getpid(), members);
        if (reserve_sem_amount(utente->shm_ptr->seat_area.semaphore_set_id, 0, members) != -1) {
            if (local_daily_cycle_is_active) {
                open_barrier_gate(utente->shm_ptr->group_sync_semaphore_id, base_sem + GROUP_SEM_TABLE_GATE);
            }
        }
    } else {
        printf("[UTENTE] PID %d: Attesa tavolo dal leader...\n", getpid());
        wait_for_zero(utente->shm_ptr->group_sync_semaphore_id, base_sem + GROUP_SEM_TABLE_GATE);
    }
}

void fase_consumazione_pasto(StatoUtente *utente, bool p1, bool p2) {
    if (!local_daily_cycle_is_active) return;
    int count = (p1?1:0) + (p2?1:0);
    if (count > 0) usleep(count * 100000);
    release_sem(utente->shm_ptr->seat_area.semaphore_set_id, 0);
    printf("[UTENTE] PID %d: Pasto terminato.\n", getpid());
}

void fase_servizio_caffe(StatoUtente *utente) {
    if (!local_daily_cycle_is_active) return;
    
    printf("[UTENTE] PID %d: Coda Caffè/Dolce...\n", getpid());
    
    /* Nota: Sez 5.2 della consegna dice che la stazione dolce/caffè ha quantità illimitata */
    /* Quindi non serve il check porzioni, solo l'interazione IPC */
    FoodDistributionStation *stazione = &utente->shm_ptr->coffee_dessert_station;
    int choice = utente->selected_dessert_coffee_index;

    /* Comunicazione IPC (Atomo 17) */
    fase_checkout_piatto(utente, stazione, &choice, 2); /* 2: Caffè */
}

void fase_uscita_collettiva(StatoUtente *utente) {
    if (utente->group_size <= 1 || !local_daily_cycle_is_active) return;
    int s_idx = utente->group_id;
    int base_sem = s_idx * GROUP_SEMS_PER_ENTRY;
    /* Usiamo NO_UNDO per le barriere di sincronizzazione serale/uscita */
    if (reserve_sem_no_undo(utente->shm_ptr->group_sync_semaphore_id, base_sem + GROUP_SEM_EXIT) != -1) {
        if (local_daily_cycle_is_active) {
            wait_for_zero(utente->shm_ptr->group_sync_semaphore_id, base_sem + GROUP_SEM_EXIT);
        }
    }
    printf("[UTENTE] PID %d: Uscita gruppo %d completata.\n", getpid(), s_idx);
}

void aggiorna_statistiche_servito(StatoUtente *utente) {
    if (!local_daily_cycle_is_active) return;
    reserve_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SIMULATION_STATS);
    utente->shm_ptr->statistics.clients_statistics.total_clients_served++;
    
    /* Statistiche Versione Completa (Punto 6 Consegna) */
    if (utente->has_ticket) {
        utente->shm_ptr->statistics.clients_statistics.total_clients_with_ticket++;
    } else {
        utente->shm_ptr->statistics.clients_statistics.total_clients_without_ticket++;
    }
    
    release_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SIMULATION_STATS);
}

void aggiorna_statistiche_non_servito(StatoUtente *utente) {
    reserve_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SIMULATION_STATS);
    utente->shm_ptr->statistics.clients_statistics.total_clients_not_served++;
    release_sem(utente->shm_ptr->semaphore_mutex_id, MUTEX_SIMULATION_STATS);
}