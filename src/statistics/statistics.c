/**
 * @file statistics.c
 * @brief Implementazione della raccolta e visualizzazione statistiche.
 * 
 * Fornisce funzioni per:
 * - Raccolta thread-safe delle statistiche dalla SHM
 * - Calcolo medie tempi di attesa
 * - Report a terminale e salvataggio su file
 * 
 * @see statistics.h per la documentazione delle strutture.
 */

/* Includes di sistema */
#include <stdio.h>
#include <string.h>

/* Includes del progetto */
#include "statistics.h"
#include "common.h"
#include "sem.h"

/* ==========================================================================
 *                         SEZIONE: RACCOLTA DATI
 * ========================================================================== */

/**
 * Raccoglie le statistiche dalla SHM con accesso protetto da mutex.
 * Calcola le medie dai dati accumulatori prima di restituire.
 */
SimulationStatistics collect_simulation_statistics(struct MainSharedMemory *shared_memory_ptr) {
    SimulationStatistics current_statistics;
    
    /* 1. Protocollo di Accesso Sicuro (Sez 5.1 Consegna) */
    reserve_sem(shared_memory_ptr->semaphore_mutex_id, MUTEX_SIMULATION_STATS);
    
    /* 2. Copia dei dati grezzi dalla SHM */
    memcpy(&current_statistics, &shared_memory_ptr->statistics, sizeof(SimulationStatistics));
    
    /* 3. Calcolo delle Medie dai dati accumulatori */
    if (current_statistics.daily_wait_accumulators.count_first > 0) {
        current_statistics.daily_average_wait_times.average_wait_first_course = 
            current_statistics.daily_wait_accumulators.sum_wait_first / current_statistics.daily_wait_accumulators.count_first;
    }
    if (current_statistics.daily_wait_accumulators.count_second > 0) {
        current_statistics.daily_average_wait_times.average_wait_second_course = 
            current_statistics.daily_wait_accumulators.sum_wait_second / current_statistics.daily_wait_accumulators.count_second;
    }
    if (current_statistics.daily_wait_accumulators.count_cashier > 0) {
        current_statistics.daily_average_wait_times.average_wait_cash_desk = 
            current_statistics.daily_wait_accumulators.sum_wait_cashier / current_statistics.daily_wait_accumulators.count_cashier;
    }
    if (current_statistics.daily_wait_accumulators.count_coffee > 0) {
        current_statistics.daily_average_wait_times.average_wait_coffee_dessert = 
            current_statistics.daily_wait_accumulators.sum_wait_coffee / current_statistics.daily_wait_accumulators.count_coffee;
    }

    /* 4. Rilascio Mutex */
    release_sem(shared_memory_ptr->semaphore_mutex_id, MUTEX_SIMULATION_STATS);

    /* Nota: Eventuali calcoli derivati (medie, percentuali) possono essere fatti qui 
       prima di restituire la struttura al chiamante. */
    
    return current_statistics;
}

/* ==========================================================================
 *                       SEZIONE: OUTPUT E REPORT
 * ========================================================================== */

/**
 * Stampa a terminale il report giornaliero formattato.
 */
void display_daily_statistics_report(SimulationStatistics statistics, int simulation_day) {
    printf("\n============================================================\n");
    printf("        REPORT GIORNALIERO - GIORNO %d\n", simulation_day + 1);
    printf("============================================================\n");
    printf("Utenti serviti oggi:       %d\n", statistics.clients_statistics.total_clients_served);
    printf("  - Con Ticket:            %d\n", statistics.clients_statistics.total_clients_with_ticket);
    printf("  - Senza Ticket:          %d\n", statistics.clients_statistics.total_clients_without_ticket);
    printf("Utenti non serviti:        %d\n", statistics.clients_statistics.total_clients_not_served);
    printf("Incasso giornaliero:       %.2f EUR\n", statistics.income_statistics.current_daily_income);
    printf("------------------------------------------------------------\n");
    printf("Dettaglio Piatti Serviti:\n");
    printf("  - Primi Piatti:          %d\n", statistics.total_served_plates.first_course_count);
    printf("  - Secondi Piatti:        %d\n", statistics.total_served_plates.second_course_count);
    printf("  - Caffè e Dessert:       %d\n", statistics.total_served_plates.coffee_dessert_count);
    printf("------------------------------------------------------------\n");
    printf("Tempi Medi di Attesa:\n");
    printf("  - Area Primi:            %.2f min\n", statistics.daily_average_wait_times.average_wait_first_course);
    printf("  - Area Secondi:          %.2f min\n", statistics.daily_average_wait_times.average_wait_second_course);
    printf("  - Cassa:                 %.2f min\n", statistics.daily_average_wait_times.average_wait_cash_desk);
    printf("============================================================\n\n");
}

/**
 * Salva le statistiche giornaliere su file.
 * Crea l'header alla prima scrittura, poi appende i report.
 */
int save_statistics_to_csv(SimulationStatistics statistics, int simulation_day, const char *filepath) {
    FILE *check_file = fopen(filepath, "r");
    int file_exists = (check_file != NULL);
    if (check_file) fclose(check_file);

    FILE *file = fopen(filepath, "a");
    if (file == NULL) {
        perror("[STATISTICS] Errore apertura file statistiche");
        return -1;
    }

    /* Header CSV con i nomi reali delle variabili C per massima coerenza */
    if (!file_exists) {
        fprintf(file, "simulation_day,"
                      "clients_statistics_total_clients_served,"
                      "clients_statistics_total_clients_not_served,"
                      "clients_statistics_total_clients_with_ticket,"
                      "clients_statistics_total_clients_without_ticket,"
                      "income_statistics_current_daily_income,"
                      "income_statistics_accumulated_total_income,"
                      "total_served_plates_first_course_count,"
                      "total_served_plates_second_course_count,"
                      "total_served_plates_coffee_dessert_count,"
                      "total_served_plates_total_plates_count,"
                      "total_leftover_plates_first_course_count,"
                      "total_leftover_plates_second_course_count,"
                      "total_leftover_plates_coffee_dessert_count,"
                      "daily_average_wait_times_average_wait_first_course,"
                      "daily_average_wait_times_average_wait_second_course,"
                      "daily_average_wait_times_average_wait_cash_desk,"
                      "daily_average_wait_times_average_wait_coffee_dessert,"
                      "total_average_wait_times_average_wait_first_course,"
                      "total_average_wait_times_average_wait_second_course,"
                      "total_average_wait_times_average_wait_cash_desk,"
                      "total_average_wait_times_average_wait_coffee_dessert,"
                      "operators_statistics_daily_active_operators,"
                      "operators_statistics_total_breaks_taken\n");
    }

    /* Scrittura riga dati */
    fprintf(file, "%d,%d,%d,%d,%d,"
                  "%.2f,%.2f,"
                  "%d,%d,%d,%d,"
                  "%d,%d,%d,"
                  "%.2f,%.2f,%.2f,%.2f,"
                  "%.2f,%.2f,%.2f,%.2f,"
                  "%d,%d\n",
            simulation_day + 1,
            statistics.clients_statistics.total_clients_served,
            statistics.clients_statistics.total_clients_not_served,
            statistics.clients_statistics.total_clients_with_ticket,
            statistics.clients_statistics.total_clients_without_ticket,
            statistics.income_statistics.current_daily_income,
            statistics.income_statistics.accumulated_total_income,
            statistics.total_served_plates.first_course_count,
            statistics.total_served_plates.second_course_count,
            statistics.total_served_plates.coffee_dessert_count,
            statistics.total_served_plates.total_plates_count,
            statistics.total_leftover_plates.first_course_count,
            statistics.total_leftover_plates.second_course_count,
            statistics.total_leftover_plates.coffee_dessert_count,
            statistics.daily_average_wait_times.average_wait_first_course,
            statistics.daily_average_wait_times.average_wait_second_course,
            statistics.daily_average_wait_times.average_wait_cash_desk,
            statistics.daily_average_wait_times.average_wait_coffee_dessert,
            statistics.total_average_wait_times.average_wait_first_course,
            statistics.total_average_wait_times.average_wait_second_course,
            statistics.total_average_wait_times.average_wait_cash_desk,
            statistics.total_average_wait_times.average_wait_coffee_dessert,
            statistics.operators_statistics.daily_active_operators,
            statistics.operators_statistics.total_breaks_taken);

    fclose(file);
    printf("[STATISTICS] Dati CSV completi giorno %d salvati in %s\n", simulation_day + 1, filepath);
    return 0;
}
