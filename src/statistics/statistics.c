#include <stdio.h>
#include <string.h>
#include "../../include/statistics.h"
#include "../../include/common.h"

/* ==========================================================================
 *                          FUNZIONI PUBBLICHE
 * ========================================================================= */

SimulationStatistics collect_simulation_statistics(struct MainSharedMemory *shared_memory_ptr) {
    SimulationStatistics current_statistics;
    
    /* Inizializzazione della struttura con zeri per garantire coerenza dei dati */
    memset(&current_statistics, 0, sizeof(SimulationStatistics));

    /* Nota: I dati verranno popolati dinamicamente durante lo sviluppo
     * della logica di servizio nelle stazioni di distribuzione e cassa. */
    
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
