/**
 * @file simulation_engine.c
 * @brief Implementazione del motore di simulazione del Master.
 * 
 * Gestisce il core loop temporale della mensa, coordinando i cicli giornalieri,
 * la sincronizzazione dei processi figli tramite barriere e i rifornimenti
 * asincroni delle stazioni.
 * 
 * @see simulation_engine.h
 */

/* Includes di sistema */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>

/* Includes del progetto */
#include "common.h"
#include "simulation_engine.h"
#include "sem.h"
#include "utils.h"
#include "statistics.h"
#include "queue.h"
#include "message.h"

/* ==========================================================================
 *                        VARIABILI GLOBALI (STATO ENGINE)
 * ========================================================================== */

/** Flag atomica per il ciclo giornaliero. */
static volatile sig_atomic_t daily_cycle_is_active = 0;

/** Flag atomica per la richiesta di refill asincrono. */
static volatile sig_atomic_t refill_requested = 0;

/** Riferimento globale alla SHM per gli handler dei segnali. */
static MainSharedMemory *global_shm_ref = NULL;

/* ==========================================================================
 *                       SEZIONE: PROTOTIPI PRIVATI
 * ========================================================================== */

static void handle_daily_cycle_end(int sig);
static void handle_emergency_termination(int sig);
static void handle_add_users_request(int sig);
static void handle_refill_signal(int sig);
static void handle_sigchld(int sig);

static void reset_daily_statistics(MainSharedMemory *shm);
static void reset_dining_area_tables(MainSharedMemory *shm);
static void flush_message_queues(MainSharedMemory *shm);
static void calculate_food_waste_and_teardown(MainSharedMemory *shm);
static void perform_initial_daily_refill(MainSharedMemory *shm);
static void process_add_users_requests(MainSharedMemory *shm);

/* ==========================================================================
 *                    SEZIONE: IMPLEMENTAZIONE PUBBLICA
 * ========================================================================== */

void run_simulation_loop(MainSharedMemory *shm) {
    printf("[MASTER] Engine in esecuzione. Avvio loop settimanale...\n");
    global_shm_ref = shm;

    /* LOOP SETTIMANALE: Gestione dei simulation_duration_days */
    while (shm->is_simulation_running && shm->current_simulation_day < shm->configuration.timings.simulation_duration_days) {
        
        /* 1. Fase Preparazione Giorno */
        int morning_barrier_ok = 0;
        int morning_critical_err = 0;
        while (shm->is_simulation_running && !morning_barrier_ok && !morning_critical_err) {
            if (wait_for_zero_interruptible(shm->semaphore_sync_id, BARRIER_MORNING_READY) == 0) {
                morning_barrier_ok = 1;
            } else if (errno != EINTR) {
                perror("[MASTER] Errore critico su barriera mattutina");
                morning_critical_err = 1;
            }
            /* EINTR: segnale ricevuto, ricontrolla is_simulation_running nel while */
        }

        if (shm->is_simulation_running) {
            printf("[MASTER] --- INIZIO GIORNO %d ---\n", shm->current_simulation_day + 1);

            reset_daily_statistics(shm);
            perform_initial_daily_refill(shm);
            setup_group_barriers(shm);
            setup_refill_signal();

            /* Setup barriera serale in base alla popolazione attuale */
            int evening_count = shm->configuration.quantities.number_of_workers +
                                shm->configuration.seats.seats_cash_desk +
                                shm->current_total_users;
            setup_barrier(shm->semaphore_sync_id, BARRIER_EVENING_READY, BARRIER_EVENING_GATE, evening_count);

            /* 2. Fase Operativa Attiva */
            daily_cycle_is_active = 1;
            reset_dining_area_tables(shm);
            flush_message_queues(shm);
            arm_daily_timer(shm);
            open_barrier_gate(shm->semaphore_sync_id, BARRIER_MORNING_GATE);

            while (daily_cycle_is_active && shm->is_simulation_running) {
                pause(); /* Attesa segnali (Timer, Refill, Emergenza) */

                if (daily_cycle_is_active && shm->is_simulation_running && refill_requested) {
                    handle_refill_cycle(shm);
                    refill_requested = 0;
                    setup_refill_signal(); /* Ri-arma per il prossimo evento casuale */
                }
            }

            /* 3. Fase Chiusura Giorno */
            if (shm->current_simulation_day + 1 >= shm->configuration.timings.simulation_duration_days) {
                shm->is_simulation_running = 0;
                shm->statistics.reason_for_termination = TERMINATION_REASON_TIMEOUT;
            }

            /* Notifica figli (Fine turno o Fine Simulazione) */
            int end_sig = (shm->is_simulation_running) ? SIGUSR2 : SIGTERM;
            broadcast_signal_to_all_groups(shm, end_sig);

            /* Sincronizzazione serale */
            int evening_barrier_ok = 0;
            int evening_critical_err = 0;
            while (shm->is_simulation_running && !evening_barrier_ok && !evening_critical_err) {
                if (wait_for_zero_interruptible(shm->semaphore_sync_id, BARRIER_EVENING_READY) == 0) {
                    evening_barrier_ok = 1;
                } else if (errno != EINTR) {
                    perror("[MASTER] Errore critico su barriera serale");
                    evening_critical_err = 1;
                }
                /* EINTR: segnale ricevuto, ricontrolla is_simulation_running nel while */
            }

            if (shm->is_simulation_running) {
                /* Elaborazione richieste add_users e preparazione barriera mattutina */
                process_add_users_requests(shm);

                open_barrier_gate(shm->semaphore_sync_id, BARRIER_EVENING_GATE);

                /* Calcolo sprechi prima del report */
                calculate_food_waste_and_teardown(shm);

                /* Reporting */
                SimulationStatistics daily_stats = collect_simulation_statistics(shm);

                /* Controllo OVERLOAD (Sez 5.6 della Consegna) */
                if (daily_stats.clients_statistics.daily_clients_not_served > shm->configuration.thresholds.overload_threshold) {
                    printf("[MASTER] TERMINAZIONE PER OVERLOAD: %d utenti rinunciatari oggi (Soglia: %d)\n",
                           daily_stats.clients_statistics.daily_clients_not_served,
                           shm->configuration.thresholds.overload_threshold);
                    shm->is_simulation_running = 0;
                    shm->statistics.reason_for_termination = TERMINATION_REASON_OVERLOAD;
                }

                display_daily_statistics_report(daily_stats, shm->current_simulation_day);
                save_statistics_to_csv(daily_stats, shm->current_simulation_day, "statistics_report.csv");

                shm->current_simulation_day++;
                printf("[MASTER] --- FINE GIORNO %d ---\n", shm->current_simulation_day);
            } else {
                open_barrier_gate(shm->semaphore_sync_id, BARRIER_EVENING_GATE);
            }
        }
    }

    /* 4. Fine Simulazione */
    usleep(500000); /* Breve attesa per permettere ai figli di stampare i log di chiusura */
    printf("\n[MASTER] Elaborazione report finale in corso...\n");
    
    SimulationStatistics final_stats = collect_simulation_statistics(shm);
    display_final_simulation_report(final_stats, shm->current_simulation_day);
}
void handle_refill_cycle(MainSharedMemory *shm) {
    /* [CONSEGNA 6] Simulazione tempo di esecuzione refill (AVG ± 20%) */
    int refill_avg = shm->configuration.timings.average_refill_time;
    int varied_refill_time = calculate_varied_time(refill_avg, 20);
    
    simulate_time_passage(varied_refill_time, shm->configuration.timings.nanoseconds_per_tick);

    /* Refill Primi */
    release_sem(shm->first_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);
    for (int i = 0; i < MAX_DISHES_PER_CATEGORY; i++) {
        shm->first_course_station.portions[i] += shm->configuration.thresholds.refill_amount_primi;
        if (shm->first_course_station.portions[i] > shm->configuration.thresholds.maximum_portions_primi) {
            shm->first_course_station.portions[i] = shm->configuration.thresholds.maximum_portions_primi;
        }
    }
    reserve_sem(shm->first_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);

    /* Refill Secondi */
    release_sem(shm->second_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);
    for (int i = 0; i < MAX_DISHES_PER_CATEGORY; i++) {
        shm->second_course_station.portions[i] += shm->configuration.thresholds.refill_amount_secondi;
        if (shm->second_course_station.portions[i] > shm->configuration.thresholds.maximum_portions_secondi) {
            shm->second_course_station.portions[i] = shm->configuration.thresholds.maximum_portions_secondi;
        }
    }
    reserve_sem(shm->second_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);

    printf("[MASTER] Refill completato in %d min.\n", varied_refill_time);
}

void arm_daily_timer(MainSharedMemory *shm) {
    struct sigevent sev;
    timer_t timerid;
    struct itimerspec its;
    struct sigaction sa;

    sa.sa_handler = handle_daily_cycle_end;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_ptr = &timerid;
    timer_create(CLOCK_REALTIME, &sev, &timerid);

    long long meal_ns = (long long)shm->configuration.timings.meal_duration_minutes * 
                         shm->configuration.timings.nanoseconds_per_tick;

    its.it_value.tv_sec = (time_t)(meal_ns / 1000000000LL);
    its.it_value.tv_nsec = (long)(meal_ns % 1000000000LL);
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    timer_settime(timerid, 0, &its, NULL);
}

void broadcast_signal_to_all_groups(MainSharedMemory *shm, int signal) {
    for (int i = 0; i < MAX_PROCESS_GROUPS; i++) {
        pid_t pgid = shm->process_group_pids[i];
        if (pgid > 0) {
            kill(-pgid, signal);
        }
    }
}

void setup_sigchld_handler(MainSharedMemory *shm) {
    struct sigaction sa;
    global_shm_ref = shm;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
}

void setup_signal_close_day(MainSharedMemory *shm) {
    struct sigaction sa;
    global_shm_ref = shm;
    sa.sa_handler = handle_emergency_termination;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    sa.sa_handler = handle_add_users_request;
    sigaction(SIGUSR1, &sa, NULL);
}

void setup_refill_signal(void) {
    struct sigevent sev;
    timer_t timerid;
    struct itimerspec its;
    struct sigaction sa;

    sa.sa_handler = handle_refill_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGRTMIN + 1, &sa, NULL);

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN + 1;
    sev.sigev_value.sival_ptr = &timerid;
    timer_create(CLOCK_REALTIME, &sev, &timerid);

    /* [CONSEGNA 5.2] Trigger refill ogni 10 minuti simulati */
    int trigger_minutes = 10;
    long long refill_ns = (long long)trigger_minutes * global_shm_ref->configuration.timings.nanoseconds_per_tick;

    its.it_value.tv_sec = (time_t)(refill_ns / 1000000000LL);
    its.it_value.tv_nsec = (long)(refill_ns % 1000000000LL);
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    timer_settime(timerid, 0, &its, NULL);
}

void setup_group_barriers(MainSharedMemory *shm_ptr) {
    for (int i = 0; i < shm_ptr->group_pool_size; i++) {
        int active = shm_ptr->group_statuses[i].active_members;
        if (active > 0) {
            int base = i * GROUP_SEMS_PER_ENTRY;
            init_sem_val(shm_ptr->group_sync_semaphore_id, base + GROUP_SEM_PRE_CASHIER, active);
            init_sem_val(shm_ptr->group_sync_semaphore_id, base + GROUP_SEM_TABLE_GATE, 1);
            init_sem_val(shm_ptr->group_sync_semaphore_id, base + GROUP_SEM_EXIT, active);
        }
    }
}

/* ==========================================================================
 *                    SEZIONE: IMPLEMENTAZIONE PRIVATA
 * ========================================================================== */

static void handle_daily_cycle_end(int sig) {
    (void)sig;
    daily_cycle_is_active = 0;
}

static void handle_emergency_termination(int sig) {
    (void)sig;
    if (global_shm_ref != NULL) {
        global_shm_ref->is_simulation_running = 0;
        global_shm_ref->statistics.reason_for_termination = TERMINATION_REASON_SIGNAL;
    }
    daily_cycle_is_active = 0;
}

static void handle_add_users_request(int sig) {
    (void)sig;
    if (global_shm_ref != NULL) global_shm_ref->add_users_flag = 1;
}

static void handle_refill_signal(int sig) {
    (void)sig;
    refill_requested = 1;
}

static void handle_sigchld(int sig) {
    (void)sig;
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (global_shm_ref != NULL) {
            /* Compensazione barriere */
            reserve_sem_try_no_undo(global_shm_ref->semaphore_sync_id, BARRIER_MORNING_READY);
            reserve_sem_try_no_undo(global_shm_ref->semaphore_sync_id, BARRIER_EVENING_READY);

            /* Compensazione gruppi */
            int found = 0;
            for (int r = 0; r < MAX_USERS_REGISTRY && !found; r++) {
                if (global_shm_ref->user_registry[r].pid == pid) {
                    int g_idx = global_shm_ref->user_registry[r].group_index;
                    int base = g_idx * GROUP_SEMS_PER_ENTRY;
                    
                    if (global_shm_ref->group_statuses[g_idx].active_members > 0) {
                        global_shm_ref->group_statuses[g_idx].active_members--;
                    }
                    
                    reserve_sem_try_no_undo(global_shm_ref->group_sync_semaphore_id, base + GROUP_SEM_PRE_CASHIER);
                    reserve_sem_try_no_undo(global_shm_ref->group_sync_semaphore_id, base + GROUP_SEM_EXIT);
                    
                    if (global_shm_ref->group_statuses[g_idx].group_leader_pid == pid) {
                        global_shm_ref->group_statuses[g_idx].group_leader_pid = 0;
                    }

                    global_shm_ref->user_registry[r].pid = 0; 
                    found = 1;
                }
            }
        }
    }
}

static void reset_daily_statistics(MainSharedMemory *shm) {
    reserve_sem(shm->semaphore_mutex_id, MUTEX_SIMULATION_STATS);
    
    /* Reset Utenti Giornalieri */
    shm->statistics.clients_statistics.daily_clients_served = 0;
    shm->statistics.clients_statistics.daily_clients_not_served = 0;
    shm->statistics.clients_statistics.daily_clients_with_ticket = 0;
    shm->statistics.clients_statistics.daily_clients_without_ticket = 0;
    
    /* Reset Piatti Giornalieri */
    memset(&shm->statistics.daily_served_plates, 0, sizeof(StatisticsPlateCounts));
    memset(&shm->statistics.daily_leftover_plates, 0, sizeof(StatisticsPlateCounts));

    /* Reset Operatori e Incassi Giornalieri */
    shm->statistics.income_statistics.current_daily_income = 0.0;
    shm->statistics.operators_statistics.daily_active_operators = 0;
    shm->statistics.operators_statistics.daily_breaks_taken = 0;
    
    /* Reset Accumulatori Tempi del Giorno */
    memset(&shm->statistics.daily_wait_accumulators, 0, sizeof(WaitTimeAccumulator));
    
    release_sem(shm->semaphore_mutex_id, MUTEX_SIMULATION_STATS);
}

/**
 * Calcola i piatti avanzati nelle stazioni alla fine della giornata.
 */
static void calculate_food_waste_and_teardown(MainSharedMemory *shm) {
    reserve_sem(shm->semaphore_mutex_id, MUTEX_SIMULATION_STATS);
    reserve_sem(shm->semaphore_mutex_id, MUTEX_SHARED_DATA);
    
    int first_waste = 0;
    for (int i = 0; i < shm->food_menu.number_of_first_courses; i++) {
        first_waste += shm->first_course_station.portions[i];
    }
    
    int second_waste = 0;
    for (int i = 0; i < shm->food_menu.number_of_second_courses; i++) {
        second_waste += shm->second_course_station.portions[i];
    }
    
    /* Caffè/Dolci: quantità illimitata (consegna sez. 5.2), non contano come waste */

    /* Aggiornamento Statistiche Giornaliere */
    shm->statistics.daily_leftover_plates.first_course_count = first_waste;
    shm->statistics.daily_leftover_plates.second_course_count = second_waste;
    shm->statistics.daily_leftover_plates.coffee_dessert_count = 0;
    shm->statistics.daily_leftover_plates.total_plates_count = first_waste + second_waste;

    /* Aggiornamento Statistiche Totali (Accumulo) */
    shm->statistics.total_leftover_plates.first_course_count += first_waste;
    shm->statistics.total_leftover_plates.second_course_count += second_waste;
    shm->statistics.total_leftover_plates.coffee_dessert_count += 0;
    shm->statistics.total_leftover_plates.total_plates_count += (first_waste + second_waste);

    release_sem(shm->semaphore_mutex_id, MUTEX_SHARED_DATA);
    release_sem(shm->semaphore_mutex_id, MUTEX_SIMULATION_STATS);
}



static void perform_initial_daily_refill(MainSharedMemory *shm) {
    /* Primi */
    release_sem(shm->first_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);
    for (int i = 0; i < MAX_DISHES_PER_CATEGORY; i++) {
        shm->first_course_station.portions[i] = shm->configuration.thresholds.refill_amount_primi;
    }
    reserve_sem(shm->first_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);

    /* Secondi */
    release_sem(shm->second_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);
    for (int i = 0; i < MAX_DISHES_PER_CATEGORY; i++) {
        shm->second_course_station.portions[i] = shm->configuration.thresholds.refill_amount_secondi;
    }
    reserve_sem(shm->second_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);

    /* Caffè e Dessert */
    release_sem(shm->coffee_dessert_station.semaphore_set_id, STATION_SEM_REFILL_GATE);
    for (int i = 0; i < 4; i++) {
        shm->coffee_dessert_station.portions[i] = 100; /* Abbondante per caffè/dolci */
    }
    reserve_sem(shm->coffee_dessert_station.semaphore_set_id, STATION_SEM_REFILL_GATE);
}

static void process_add_users_requests(MainSharedMemory *shm) {
    int processed = 0;
    printf("[DEBUG-MASTER] process_add_users_requests: add_users_flag=%d, current_total_users=%d\n",
           shm->add_users_flag, shm->current_total_users);

    if (shm->add_users_flag) {
        SimulationMessage msg;
        while (receive_message_from_queue(shm->control_queue_id, &msg, sizeof(ControlPayload), 0, IPC_NOWAIT) != -1) {
            processed++;
            /* NON incrementiamo current_total_users qui - lo farà add_users dopo lo spawn */
        }
        printf("[DEBUG-MASTER] Letti %d messaggi dalla coda\n", processed);
    }

    if (processed > 0) {
        shm->add_users_flag = 0;

        printf("[DEBUG-MASTER] Configuro BARRIER_ADD_USERS con count=%d\n", processed);
        setup_barrier(shm->semaphore_sync_id, BARRIER_ADD_USERS_READY, BARRIER_ADD_USERS_GATE, processed);

        printf("[DEBUG-MASTER] Rilascio %d permessi MUTEX_ADD_USERS_PERMISSION\n", processed);
        for (int i = 0; i < processed; i++) {
            release_sem(shm->semaphore_mutex_id, MUTEX_ADD_USERS_PERMISSION);
        }

        printf("[DEBUG-MASTER] Attendo BARRIER_ADD_USERS_READY = 0...\n");
        int add_users_barrier_ok = 0;
        int add_users_critical_err = 0;
        while (shm->is_simulation_running && !add_users_barrier_ok && !add_users_critical_err) {
            if (wait_for_zero_interruptible(shm->semaphore_sync_id, BARRIER_ADD_USERS_READY) == 0) {
                add_users_barrier_ok = 1;
            } else if (errno != EINTR) {
                perror("[MASTER] Errore critico su barriera add_users");
                add_users_critical_err = 1;
            }
            /* EINTR: segnale ricevuto, ricontrolla is_simulation_running nel while */
        }
        printf("[DEBUG-MASTER] BARRIER_ADD_USERS_READY raggiunto 0, current_total_users=%d\n",
               shm->current_total_users);
    }

    /* Configura la barriera mattutina PRIMA di aprire i gate - così è pronta quando i nuovi utenti partono */
    int next_morning_count = shm->configuration.quantities.number_of_workers +
                             shm->configuration.seats.seats_cash_desk +
                             shm->current_total_users;
    printf("[DEBUG-MASTER] Configuro BARRIER_MORNING: workers=%d + casse=%d + users=%d = %d\n",
           shm->configuration.quantities.number_of_workers,
           shm->configuration.seats.seats_cash_desk,
           shm->current_total_users,
           next_morning_count);
    setup_barrier(shm->semaphore_sync_id, BARRIER_MORNING_READY, BARRIER_MORNING_GATE, next_morning_count);

    if (processed > 0) {
        printf("[DEBUG-MASTER] Apro BARRIER_ADD_USERS_GATE\n");
        open_barrier_gate(shm->semaphore_sync_id, BARRIER_ADD_USERS_GATE);
        printf("[MASTER] Elaborati %d blocchi add_users. Spawn completato.\n", processed);
    }
    shm->add_users_flag = 0;
}

/**
 * Resetta i posti occupati di tutti i tavoli a 0 all'inizio di ogni giornata.
 * Risolve il leak di posti causato da utenti interrotti prima di liberare il tavolo.
 */
static void reset_dining_area_tables(MainSharedMemory *shm) {
    reserve_sem(shm->semaphore_mutex_id, MUTEX_TABLES);
    for (int i = 0; i < shm->seat_area.active_tables_count; i++) {
        shm->seat_area.tables[i].occupied_seats = 0;
    }
    release_sem(shm->semaphore_mutex_id, MUTEX_TABLES);
}

/**
 * Svuota tutte le code di messaggi delle stazioni all'inizio di ogni giornata.
 * Risolve l'accumulo di messaggi orfani (ordini non processati del giorno precedente).
 */
static void flush_message_queues(MainSharedMemory *shm) {
    SimulationMessage msg;
    int flushed_count = 0;

    /* Svuota coda Primi Piatti */
    while (receive_message_from_queue(shm->first_course_station.message_queue_id,
           &msg, sizeof(StationPayload), 0, IPC_NOWAIT) != -1) {
        flushed_count++;
    }

    /* Svuota coda Secondi Piatti */
    while (receive_message_from_queue(shm->second_course_station.message_queue_id,
           &msg, sizeof(StationPayload), 0, IPC_NOWAIT) != -1) {
        flushed_count++;
    }

    /* Svuota coda Caffè/Dolci */
    while (receive_message_from_queue(shm->coffee_dessert_station.message_queue_id,
           &msg, sizeof(StationPayload), 0, IPC_NOWAIT) != -1) {
        flushed_count++;
    }

    /* Svuota coda Cassa */
    while (receive_message_from_queue(shm->register_station.message_queue_id,
           &msg, sizeof(CashierPayload), 0, IPC_NOWAIT) != -1) {
        flushed_count++;
    }

    if (flushed_count > 0) {
        printf("[MASTER] Svuotate code messaggi: %d messaggi orfani rimossi.\n", flushed_count);
    }
}
