/**
 * @file simulation_engine.c
 * @brief Implementazione del motore di simulazione.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include "common.h"
#include "simulation_engine.h"
#include "sem.h"
#include "utils.h"
#include "statistics.h"
#include "queue.h"
#include "message.h"

static volatile sig_atomic_t daily_cycle_is_active = 0;
static volatile sig_atomic_t refill_requested = 0;
static MainSharedMemory *global_shm_ref = NULL;

/* ==========================================================================
 *                          FUNZIONI PUBBLICHE
 * ========================================================================= */

void run_simulation_loop(MainSharedMemory *shm) {
    printf("[MASTER] Engine caricato. In attesa del comando di inizio ciclo...\n");
    global_shm_ref = shm;

    /* Modifica condizione loop: includere simulation_duration_days (<= invece di <) secondo richiesta utente */
    /* Ciclo giorni: esegue esattamente simulation_duration_days (es. 0 e 1 se duration=2) */
    while (shm->is_simulation_running && shm->current_simulation_day < shm->configuration.timings.simulation_duration_days) {
        /* Attesa che tutti i figli siano pronti per l'inizio del giorno (Robusta contro EINTR) */
        while (wait_for_zero(shm->semaphore_sync_id, BARRIER_MORNING_READY) == -1 && errno == EINTR);
        printf("[MASTER] Tutti i processi pronti per il giorno %d.\n", shm->current_simulation_day + 1);

        /* [RESET STATS] Azzeramento contatori giornalieri (Sez 6.1) */
        reserve_sem(shm->semaphore_mutex_id, MUTEX_SIMULATION_STATS);
        shm->statistics.clients_statistics.total_clients_served = 0;
        shm->statistics.clients_statistics.total_clients_not_served = 0;
        shm->statistics.clients_statistics.total_clients_with_ticket = 0;
        shm->statistics.clients_statistics.total_clients_without_ticket = 0;
        
        shm->statistics.total_served_plates.first_course_count = 0;
        shm->statistics.total_served_plates.second_course_count = 0;
        shm->statistics.total_served_plates.coffee_dessert_count = 0;

        shm->statistics.income_statistics.current_daily_income = 0.0;
        shm->statistics.operators_statistics.daily_active_operators = 0;
        
        /* Reset accumulatori tempi di attesa */
        shm->statistics.daily_wait_accumulators.sum_wait_first = 0;
        shm->statistics.daily_wait_accumulators.count_first = 0;
        shm->statistics.daily_wait_accumulators.sum_wait_second = 0;
        shm->statistics.daily_wait_accumulators.count_second = 0;
        shm->statistics.daily_wait_accumulators.sum_wait_cashier = 0;
        shm->statistics.daily_wait_accumulators.count_cashier = 0;
        shm->statistics.daily_wait_accumulators.sum_wait_coffee = 0;
        shm->statistics.daily_wait_accumulators.count_coffee = 0;
        
        release_sem(shm->semaphore_mutex_id, MUTEX_SIMULATION_STATS);

        /* Avvio del ciclo giornaliero */
        daily_cycle_is_active = 1;
        arm_daily_timer(shm);

        /* [INIZIO GIORNO] Inizializzazione Stazioni (Punto 5.2):
           Le stazioni vengono riempite con AVG_REFILL all'inizio di ogni giorno.
           Senza questo, gli utenti trovano tutto vuoto fino al primo refill casuale. 
           NOTA: Il pattern per il GATE è release(+1 = chiudi/occupato), modifica, reserve(-1 = apri/libero, torna a 0). */
        
        /* PRIMI */
        release_sem(shm->first_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);
        for (int i = 0; i < MAX_DISHES_PER_CATEGORY; i++) {
            shm->first_course_station.portions[i] = shm->configuration.thresholds.refill_amount_primi;
        }
        reserve_sem(shm->first_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);

        /* SECONDI */
        release_sem(shm->second_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);
        for (int i = 0; i < MAX_DISHES_PER_CATEGORY; i++) {
            shm->second_course_station.portions[i] = shm->configuration.thresholds.refill_amount_secondi;
        }
        reserve_sem(shm->second_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);
        printf("[MASTER] Stazioni rifornite per inizio giornata.\n");

        /* 107. Setup barriera della sera (Operatori + Cassieri + Utenti Correnti) */
        int evening_process_count = shm->configuration.quantities.number_of_workers + 
                                   shm->configuration.seats.seats_cash_desk + 
                                   shm->current_total_users;
        setup_barrier(shm->semaphore_sync_id, BARRIER_EVENING_READY, BARRIER_EVENING_GATE, evening_process_count);

        /* 110. Setup timer refill casuale (5-60 min sim) */
        setup_refill_signal();

        /* [GRUPPI] Reset barriere sincronizzazione amici */
        setup_group_barriers(shm);

        /* Apertura del cancello mattutino: via libera ai figli */
        open_barrier_gate(shm->semaphore_sync_id, BARRIER_MORNING_GATE);
        printf("[MASTER] Giorno %d: Cancello aperto. Inizio attività.\n", shm->current_simulation_day + 1);

        while (daily_cycle_is_active && shm->is_simulation_running) {
            pause();

            /* Gestione Refill (Atomo 1, L109) */
            if (refill_requested && shm->is_simulation_running) {
                handle_refill_cycle(shm);
                refill_requested = 0;
                setup_refill_signal(); /* Ri-arma il prossimo refill */
            }
        }

        /* Verifica se il giorno appena concluso era l'ultimo programmato (Atomo 3.1) */
        if (shm->current_simulation_day + 1 >= shm->configuration.timings.simulation_duration_days) {
            shm->is_simulation_running = 0;
        }

        /* 117. Notifica fine giornata (SIGUSR2) o terminazione (SIGTERM) a tutti i figli */
        int end_signal = (shm->is_simulation_running) ? SIGUSR2 : SIGTERM;
        broadcast_signal_to_all_groups(shm, end_signal);

        /* 118. Sincronizzazione serale: attesa robusta (EINTR) dello zero sulla barriera */
        while (wait_for_zero(shm->semaphore_sync_id, BARRIER_EVENING_READY) == -1 && errno == EINTR);

        /* Se la simulazione è ancora attiva, prepariamo il giorno successivo */
        if (shm->is_simulation_running) {
            /* 120. Gestione atomica Add Users (Punto 3) - Eseguita a fine giornata */
            int requests_processed = 0;
            if (shm->add_users_flag) {
                printf("[MASTER] Elaborazione richieste Add Users in sospeso...\n");
                SimulationMessage ctrl_msg;
                while (receive_message_from_queue(shm->control_queue_id, &ctrl_msg, sizeof(ControlPayload), 0, IPC_NOWAIT) != -1) {
                    requests_processed++;
                    ControlPayload *payload = (ControlPayload *)ctrl_msg.message_text;
                    
                    if (shm->current_total_users + payload->users_count > MAX_USERS_REGISTRY) {
                        fprintf(stderr, "[WARNING] Superato limite massimo utenti (%d). Troncamento.\n", MAX_USERS_REGISTRY);
                        shm->current_total_users = MAX_USERS_REGISTRY;
                        break;
                    }
                    shm->current_total_users += payload->users_count;
                }
            }

            /* 125. Reset Barriera Mattutina per il giorno successivo (PUNTO 1) */
            int morning_process_count = shm->configuration.quantities.number_of_workers + 
                                       shm->configuration.seats.seats_cash_desk + 
                                       shm->current_total_users;
            setup_barrier(shm->semaphore_sync_id, BARRIER_MORNING_READY, BARRIER_MORNING_GATE, morning_process_count);
            
            /* 127. Rilascio permesso per utility add_users (Atomo 1) */
            if (requests_processed > 0) {
                shm->add_users_flag = 0;
                for (int i = 0; i < requests_processed; i++) {
                    release_sem(shm->semaphore_mutex_id, MUTEX_ADD_USERS_PERMISSION);
                }
                printf("[MASTER] Inviati %d permessi di spawn all'utility.\n", requests_processed);
            }

            /* 130. Apertura cancello serale: via libera ai figli per il riposo */
            open_barrier_gate(shm->semaphore_sync_id, BARRIER_EVENING_GATE);

            /* [REPORT] Raccolta e visualizzazione statistiche giornaliere */
            SimulationStatistics daily_stats = collect_simulation_statistics(shm);
            display_daily_statistics_report(daily_stats, shm->current_simulation_day);
            save_statistics_to_csv(daily_stats, shm->current_simulation_day, "statistics_report.txt");

            /* Avanzamento della simulazione al giorno successivo */
            shm->current_simulation_day++;
            printf("[MASTER] Giorno concluso correttamente.\n");
        } else {
            /* Se stiamo chiudendo tutto, assicuriamoci di sbloccare eventuali figli rimasti */
            open_barrier_gate(shm->semaphore_sync_id, BARRIER_EVENING_GATE);
        }
    }

    /* [REPORT FINALE] Visualizzazione riepilogo a fine simulazione */
    printf("\n--- SIMULAZIONE COMPLETATA ---\n");
    
    if (shm->current_simulation_day >= shm->configuration.timings.simulation_duration_days) {
         printf("[INFO] Raggiunta la durata massima configurata (%d giorni).\n", shm->configuration.timings.simulation_duration_days);
    } else {
        /* Se interrotta prematuramente dal Master (es. segnale esterno) */
        SimulationStatistics final_stats = collect_simulation_statistics(shm);
        printf("[INFO] Simulazione Arrestata (Giorno %d).\n", shm->current_simulation_day + 1);
        display_daily_statistics_report(final_stats, shm->current_simulation_day);
    }

    printf("[MASTER] Engine terminato.\n");
}

/* ==========================================================================
 *                          FUNZIONI PRIVATE / HELPER
 * ========================================================================= */

/**
 * @brief Handler per la fine della giornata lavorativa.
 */
static void handle_daily_cycle_end(int sig) {
    (void)sig;
    daily_cycle_is_active = 0;
}

/**
 * @brief Handler per terminazione d'emergenza o superamento soglie.
 */
static void handle_emergency_termination(int sig) {
    if (global_shm_ref != NULL) {
        global_shm_ref->is_simulation_running = 0;
    }
    daily_cycle_is_active = 0;
    printf("\n[MASTER] Ricevuto segnale %d: Arresto della simulazione in corso...\n", sig);
}

/**
 * @brief Handler per la richiesta di aggiunta utenti (SIGUSR1).
 * Notifica solo la presenza di nuovi utenti senza interrompere la giornata.
 */
static void handle_add_users_request(int sig) {
    (void)sig;
    if (global_shm_ref != NULL) {
        global_shm_ref->add_users_flag = 1;
    }
}

/**
 * @brief Handler per la richiesta di refill.
 */
static void handle_refill_signal(int sig) {
    (void)sig;
    refill_requested = 1;
}

void handle_refill_cycle(MainSharedMemory *shm) {
    printf("[MASTER] Avvio ciclo di rifornimento stazioni...\n");

    /* --- Refill Primi --- */
    /* Chiude il cancello: release_sem(+1) porta il valore a 1, bloccando wait_for_zero degli operatori */
    release_sem(shm->first_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);
    
    for (int i = 0; i < MAX_DISHES_PER_CATEGORY; i++) {
        shm->first_course_station.portions[i] += shm->configuration.thresholds.refill_amount_primi;
        if (shm->first_course_station.portions[i] > shm->configuration.thresholds.maximum_portions_primi) {
            shm->first_course_station.portions[i] = shm->configuration.thresholds.maximum_portions_primi;
        }
    }
    /* Riapre il cancello: reserve_sem(-1) porta il valore a 0, sbloccando wait_for_zero */
    reserve_sem(shm->first_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);


    /* --- Refill Secondi --- */
    /* Chiude il cancello: release_sem(+1) porta il valore a 1 */
    release_sem(shm->second_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);

    for (int i = 0; i < MAX_DISHES_PER_CATEGORY; i++) {
        shm->second_course_station.portions[i] += shm->configuration.thresholds.refill_amount_secondi;
        if (shm->second_course_station.portions[i] > shm->configuration.thresholds.maximum_portions_secondi) {
            shm->second_course_station.portions[i] = shm->configuration.thresholds.maximum_portions_secondi;
        }
    }
    /* Riapre il cancello: reserve_sem(-1) porta il valore a 0 */
    reserve_sem(shm->second_course_station.semaphore_set_id, STATION_SEM_REFILL_GATE);

    printf("[MASTER] Rifornimento completato.\n");
}

void arm_daily_timer(MainSharedMemory *shm) {
    struct sigevent sev;
    timer_t timerid;
    struct itimerspec its;
    struct sigaction sa;

    /* Configura l'handler per SIGALRM */
    sa.sa_handler = handle_daily_cycle_end;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    /* Crea il timer */
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_ptr = &timerid;
    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) {
        perror("[ERROR] timer_create fallito");
        return;
    }

    /* Calcolo durata reale: (minuti_simulati * nanosecs_per_tick) */
    long long meal_ns = (long long)shm->configuration.timings.meal_duration_minutes * 
                         shm->configuration.timings.nanoseconds_per_tick;

    /* Imposta la durata reale per il timer POSIX */
    its.it_value.tv_sec = (time_t)(meal_ns / 1000000000LL);
    its.it_value.tv_nsec = (long)(meal_ns % 1000000000LL);
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("[ERROR] timer_settime fallito");
    }
}

void broadcast_signal_to_all_groups(MainSharedMemory *shm, int signal) {
    for (int i = 0; i < MAX_PROCESS_GROUPS; i++) {
        pid_t pgid = shm->process_group_pids[i];
        if (pgid > 0) {
            /* Invia il segnale a tutto il gruppo di processi (PGID) */
            if (kill(-pgid, signal) == -1) {
                /* Non è un errore critico se un gruppo è già terminato */
            }
        }
    }
    printf("[MASTER] Notifica di fine ciclo inviata a tutti i gruppi (Segnale: %d)\n", signal);
}

/**
 * @brief Handler per la morte asincrona dei processi figli.
 * Esegue la compensazione delle barriere per evitare deadlock.
 */
static void handle_sigchld(int sig) {
    (void)sig;
    int status;
    pid_t pid;

    /* Pulisce tutti i processi zombie */
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (global_shm_ref != NULL) {
            /* 1. Compensazione barriere globali (Morning/Evening) */
            reserve_sem_try_no_undo(global_shm_ref->semaphore_sync_id, BARRIER_MORNING_READY);
            reserve_sem_try_no_undo(global_shm_ref->semaphore_sync_id, BARRIER_EVENING_READY);

            /* 2. Compensazione barriere di gruppo (Logica Anti-Deadlock Proposta 2) */
            for (int r = 0; r < MAX_USERS_REGISTRY; r++) {
                if (global_shm_ref->user_registry[r].pid == pid) {
                    int g_idx = global_shm_ref->user_registry[r].group_index;
                    int base = g_idx * GROUP_SEMS_PER_ENTRY;
                    
                    /* [BUG #6 FIX] Decrementa il contatore membri attivi del gruppo.
                       NOTA: Accesso senza mutex intenzionale (safe-by-design):
                       - Il processo proprietario (pid) è già terminato
                       - Nessun altro processo può modificare questi dati specifici
                       - Usare semop in signal handler causerebbe deadlock */
                    if (global_shm_ref->group_statuses[g_idx].active_members > 0) {
                        global_shm_ref->group_statuses[g_idx].active_members--;
                    }
                    
                    /* Tenta di sbloccare le barriere di gruppo se il processo è morto prematuramente.
                       Usiamo reserve_sem_try_no_undo per non bloccare l'handler del Master. */
                    reserve_sem_try_no_undo(global_shm_ref->group_sync_semaphore_id, base + GROUP_SEM_PRE_CASHIER);
                    reserve_sem_try_no_undo(global_shm_ref->group_sync_semaphore_id, base + GROUP_SEM_EXIT);
                    
                    /* [BUG #5 FIX] Successione Leadership: se muore il leader, liberiamo il posto.
                       Accesso senza mutex safe-by-design (vedi nota sopra). */
                    if (global_shm_ref->group_statuses[g_idx].group_leader_pid == pid) {
                        global_shm_ref->group_statuses[g_idx].group_leader_pid = 0;
                    }

                    /* Libera lo slot per futuri utenti (es. Add_Users) */
                    global_shm_ref->user_registry[r].pid = 0; 
                    break;
                }
            }
        }
    }
}

void setup_sigchld_handler(MainSharedMemory *shm) {
    struct sigaction sa;
    global_shm_ref = shm;

    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP; /* Non ci interessano i figli sospesi, solo terminati */
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("[ERROR] sigaction SIGCHLD fallita");
    }
}

void setup_signal_close_day(MainSharedMemory *shm) {
    struct sigaction sa;
    global_shm_ref = shm;

    sa.sa_handler = handle_emergency_termination;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    /* Gestione Terminazione (Ctrl+C o segnali di sistema) */
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Gestione Overload e Richieste dinamiche (SIGUSR1) */
    /* NOTA: SIGUSR1 ora gestisce la richiesta asincrona di add_users senza terminare la giornata */
    sa.sa_handler = handle_add_users_request;
    sigaction(SIGUSR1, &sa, NULL);
}

void setup_refill_signal(void) {
    struct sigevent sev;
    timer_t timerid;
    struct itimerspec its;
    struct sigaction sa;

    /* Configura l'handler per SIGRTMIN + 1 (Refill Interno Master) */
    sa.sa_handler = handle_refill_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGRTMIN + 1, &sa, NULL);

    /* Crea il timer */
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN + 1;
    sev.sigev_value.sival_ptr = &timerid;
    timer_create(CLOCK_REALTIME, &sev, &timerid);

    /* Calcola intervallo casuale (5-60 minuti simulati) */
    int random_minutes = generate_random_integer(5, 60);
    long long refill_ns = (long long)random_minutes * global_shm_ref->configuration.timings.nanoseconds_per_tick;

    /* Imposta la durata reale per il timer POSIX */
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
            /* Le barriere attendono che tutti i membri attivi facciano reserve_sem (arrivino a zero) */
            init_sem_val(shm_ptr->group_sync_semaphore_id, base + GROUP_SEM_PRE_CASHIER, active);
            init_sem_val(shm_ptr->group_sync_semaphore_id, base + GROUP_SEM_TABLE_GATE, 1); /* Chiuso (aperto dal leader) */
            init_sem_val(shm_ptr->group_sync_semaphore_id, base + GROUP_SEM_EXIT, active);
        }
    }
}
