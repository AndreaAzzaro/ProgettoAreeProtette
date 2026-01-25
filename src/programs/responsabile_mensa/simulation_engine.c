/**
 * @file simulation_engine.c
 * @brief Implementazione del ciclo principale della simulazione.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "common.h"
#include "simulation_engine.h"
#include "setup_population.h"
#include "sem.h"
#include "signals_handler.h"
#include <signal.h>
#include <time.h>
#include <errno.h>

/**
 * @brief Variabili atomiche per la comunicazione tra Signal Handlers e Loop Principale.
 * Garantiscono thread-safety e visibilità immediata.
 */
static volatile sig_atomic_t day_in_progress = 0;
static volatile sig_atomic_t event_trigger = 0;

/**
 * @brief Handler dei segnali per i timer del Master.
 * Legge l'ID dell'evento dal segnale e aggiorna le variabili di stato.
 */
/* Identificatori univoci per gli eventi generati dai timer */
#define EVENT_NONE 0
#define EVENT_REFILL 1
#define EVENT_DAY_END 2

/**
 * @brief Handler dei segnali per i timer del Master.
 * Legge l'ID dell'evento dal segnale e aggiorna le variabili di stato.
 */
static void master_timer_handler(int sig, siginfo_t *si, void *uc) {
    (void)sig; (void)uc;
    int event_id = si->si_value.sival_int;
    
    if (event_id == EVENT_REFILL) {
        event_trigger = EVENT_REFILL;
    } else if (event_id == EVENT_DAY_END) {
        event_trigger = EVENT_DAY_END;
        day_in_progress = 0;
    }
}

/**
 * @brief Crea un timer POSIX che invia un segnale SIGRTMIN con un ID evento.
 * NOTA ARCHITETTURALE: Utilizziamo i POSIX Timers (invece di alarm() o sleep()) perché:
 * 1. Permettono di gestire eventi multipli (Refill + Fine Giornata) con un unico segnale.
 * 2. Sono più precisi e non interrompono forzatamente il processo in modo incontrollato.
 * 3. Lo standard System-V non fornisce timer/allarmi nativi; POSIX è lo standard per il timing.
 * Le risorse di comunicazione (SHM, SEM, MSG) rimangono rigorosamente System-V.
 */
static timer_t create_timer(int event_id) {
    timer_t timerid;
    struct sigevent sev;
    
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_int = event_id;

    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) {
        perror("[ERROR] timer_create failed");
        exit(EXIT_FAILURE);
    }
    return timerid;
}

/**
 * @brief Configura e avvia un timer (one-shot o periodico).
 * @param seconds Secondi di attesa.
 * @param nanoseconds Nanosecondi di attesa.
 * @param periodic 1 per timer periodico, 0 per singolo scatto.
 */
static void start_timer(timer_t timerid, long seconds, long nanoseconds, int periodic) {
    struct itimerspec its;
    
    /* Tempo del primo scatto */
    its.it_value.tv_sec = seconds;
    its.it_value.tv_nsec = nanoseconds;
    
    /* Intervallo per ripetizioni (se periodic=1) */
    if (periodic) {
        its.it_interval.tv_sec = seconds;
        its.it_interval.tv_nsec = nanoseconds;
    } else {
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;
    }

    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("[ERROR] timer_settime failed");
    }
}

void handle_new_day(MainSharedMemory *shm) {
    shm->current_simulation_day++;
    shm->simulation_minutes_passed = 0;
    
    /* Reset statistiche giornaliere */
    shm->register_station.daily_income = 0;
    
    printf("\n[SYSTEM] --- Inizio Giorno %d ---\n", shm->current_simulation_day);
}

void handle_refill_cycle(MainSharedMemory *shm) {
    (void)shm;
    /* TODO: Implementazione logica refill dinamico */
    printf("[SYSTEM] Rifornimento in corso...\n");
}

#include <sys/msg.h>

/**
 * @brief Interroga le code di messaggi per contare quanti utenti sono ancora in attesa.
 * Utilizza msgctl con IPC_STAT per ottenere il numero di messaggi presenti.
 */
static int get_users_in_queue_count(MainSharedMemory *shm) {
    struct msqid_ds ds;
    int total_users = 0;
    int queues[] = {
        shm->first_course_station.message_queue_id,
        shm->second_course_station.message_queue_id,
        shm->coffee_dessert_station.message_queue_id
    };

    for (int i = 0; i < 3; i++) {
        if (msgctl(queues[i], IPC_STAT, &ds) != -1) {
            total_users += ds.msg_qnum; // msq_qnum è il numero di messaggi correnti
        }
    }
    return total_users;
}

void run_simulation_loop(MainSharedMemory *shm) {
    sigset_t mask, oldmask;
    struct sigaction sa;

    /* 1. Setup Maschere di Segnale */
    sigemptyset(&mask);
    sigaddset(&mask, SIGRTMIN);

    if (sigprocmask(SIG_BLOCK, &mask, &oldmask) == -1) {
        perror("[ERROR] sigprocmask failed");
        return;
    }

    /* 2. Configurazione Handler (SA_SIGINFO permette di ricevere dati extra) */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = master_timer_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGRTMIN, &sa, NULL) == -1) {
        perror("[ERROR] sigaction failed");
        return;
    }

    /* 3. Creazione dei Timer */
    timer_t refill_timer = create_timer(EVENT_REFILL);
    timer_t end_day_timer = create_timer(EVENT_DAY_END);

    /* Setup Iniziale Barriera Mattutina */
    int total_p = shm->configuration.quantities.number_of_workers + 2 + shm->configuration.quantities.number_of_initial_users;
    setup_barrier(shm->semaphore_sync_id, BARRIER_MORNING_READY, BARRIER_MORNING_GATE, total_p);

    while (shm->is_simulation_running && shm->current_simulation_day < shm->configuration.timings.simulation_duration_days) {
        handle_new_day(shm);
        open_barrier_gate(shm->semaphore_sync_id, BARRIER_MORNING_GATE);

        /* Setup anticipato barriera sera */
        setup_barrier(shm->semaphore_sync_id, BARRIER_EVENING_READY, BARRIER_EVENING_GATE, total_p);

        printf("[SYSTEM] --- Inizio Giorno %d ---\n", shm->current_simulation_day);

        /* 4. Avvio Timer Giornalieri */
        // Refill periodico (ogni 10 min simulati)
        long refill_ns = 10 * shm->configuration.timings.nanoseconds_per_tick;
        start_timer(refill_timer, 0, refill_ns, 1);
        
        // Fine giornata (dopo la durata del pasto configurata)
        long day_ns = shm->configuration.timings.meal_duration_minutes * shm->configuration.timings.nanoseconds_per_tick;
        start_timer(end_day_timer, 0, day_ns, 0);

        day_in_progress = 1;

        /* 5. Loop di Attesa Passiva (Event-Driven) */
        while (day_in_progress && shm->is_simulation_running) {
            /* 
             * Il Master dorme qui. sigsuspend sblocca temporaneamente SIGRTMIN
             * e rimette il processo a dormire appena l'handler ha finito.
             */
            sigsuspend(&oldmask);

            if (event_trigger == EVENT_REFILL) {
                handle_refill_cycle(shm);
                event_trigger = EVENT_NONE;
            }
            // EVENT_DAY_END resetta day_in_progress nell'handler
        }

        /* 6. Chiusura Giornata */
        start_timer(refill_timer, 0, 0, 0); // Stop timer refill
        
        printf("[SYSTEM] Orario di chiusura raggiunto. Attesa sblocco barriere...\n");
        open_barrier_gate(shm->semaphore_sync_id, BARRIER_EVENING_GATE);

        /* ------------------------------------------------------------
         * 7. VERIFICA OVERLOAD (CONSEGNA RIGA 246)
         * Se troppi utenti sono rimasti bloccati in coda, la simulazione
         * termina immediatamente per ragioni di ordine pubblico.
         * ------------------------------------------------------------ */
        int final_queue_count = get_users_in_queue_count(shm);
        if (final_queue_count > shm->configuration.thresholds.overload_threshold) {
             printf("[CRITICAL] OVERLOAD RILEVATO: %d utenti ancora in coda (Soglia: %d)!\n", 
                     final_queue_count, shm->configuration.thresholds.overload_threshold);
             printf("[SYSTEM] Terminazione forzata della simulazione per disordine.\n");
             shm->is_simulation_running = 0;
        }

        /* 8. Report Giornaliero e Incremento Giorno */
        printf("[STATS] Giorno %d concluso.\n", shm->current_simulation_day + 1);
        printf("        - Incasso Giornaliero: %.2f EUR\n", shm->register_station.daily_income);
        
        shm->current_simulation_day++;
        
        /* Se la simulazione continua, resettiamo le barriere per domani */
        if (shm->is_simulation_running && shm->current_simulation_day < shm->configuration.timings.simulation_duration_days) {
            setup_barrier(shm->semaphore_sync_id, BARRIER_MORNING_READY, BARRIER_MORNING_GATE, total_p);
        }
    }

    /* 3. Cleanup dei Timer e Ripristino Maschera */
    timer_delete(refill_timer);
    timer_delete(end_day_timer);
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
}
