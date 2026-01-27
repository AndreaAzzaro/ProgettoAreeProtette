#include <stdio.h>
#include <string.h>
#include "statistics.h"
#include "common.h"
#include "sem.h"

/* ==========================================================================
 *                          FUNZIONI PUBBLICHE
 * ========================================================================= */

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

int save_statistics_to_csv(SimulationStatistics statistics, int simulation_day, const char *filepath) {
    /* Verifica se il file esiste già per decidere se scrivere l'header */
    FILE *check_file = fopen(filepath, "r");
    int file_exists = (check_file != NULL);
    if (check_file) fclose(check_file);

    /* Apri in modalità append */
    FILE *file = fopen(filepath, "a");
    if (file == NULL) {
        perror("[STATISTICS] Errore apertura file statistiche");
        return -1;
    }

    /* Scrivi header solo se il file non esisteva */
    if (!file_exists) {
        fprintf(file, "=============================================================\n");
        fprintf(file, "          STATISTICHE SIMULAZIONE MENSA\n");
        fprintf(file, "=============================================================\n\n");
    }

    /* Scrivi report giornaliero formattato */
    fprintf(file, "--- Giorno %d ---\n", simulation_day + 1);
    fprintf(file, "Clienti Serviti: %d\n", statistics.clients_statistics.total_clients_served);
    fprintf(file, "  - Con Ticket: %d\n", statistics.clients_statistics.total_clients_with_ticket);
    fprintf(file, "  - Senza Ticket: %d\n", statistics.clients_statistics.total_clients_without_ticket);
    fprintf(file, "Clienti Non Serviti: %d\n", statistics.clients_statistics.total_clients_not_served);
    fprintf(file, "Incasso Giornaliero: %.2f EUR\n", statistics.income_statistics.current_daily_income);
    fprintf(file, "Piatti Serviti:\n");
    fprintf(file, "  - Primi: %d\n", statistics.total_served_plates.first_course_count);
    fprintf(file, "  - Secondi: %d\n", statistics.total_served_plates.second_course_count);
    fprintf(file, "  - Caffe/Dessert: %d\n", statistics.total_served_plates.coffee_dessert_count);
    fprintf(file, "Tempi Attesa Medi:\n");
    fprintf(file, "  - Primi: %.2f min\n", statistics.daily_average_wait_times.average_wait_first_course);
    fprintf(file, "  - Secondi: %.2f min\n", statistics.daily_average_wait_times.average_wait_second_course);
    fprintf(file, "  - Cassa: %.2f min\n", statistics.daily_average_wait_times.average_wait_cash_desk);
    fprintf(file, "\n");

    fclose(file);
    printf("[STATISTICS] Statistiche giorno %d salvate in %s\n", simulation_day + 1, filepath);
    return 0;
}
