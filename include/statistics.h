/**
 * @file statistics.h
 * @brief Definizioni per la raccolta e visualizzazione delle statistiche di simulazione.
 */

#ifndef STATISTICS_H
#define STATISTICS_H

/** Forward declaration per evitare dipendenze circolari con common.h */
struct MainSharedMemory;

/**
 * @brief Identificatori per le cause di terminazione della simulazione.
 */
typedef enum {
    TERMINATION_REASON_NOT_TERMINATED = 0,
    TERMINATION_REASON_TIMEOUT,          /**< Raggiunta durata SIMURATION_DAYS */
    TERMINATION_REASON_OVERLOAD,         /**< Numero utenti in attesa > OVERLOAD_THRESHOLD */
    TERMINATION_REASON_SIGNAL            /**< Terminazione manuale via segnale (es. SIGINT) */
} TerminationReason;

/**
 * @brief Conteggio dei piatti serviti o avanzati.
 */
typedef struct {
    int first_course_count;
    int second_course_count;
    int coffee_dessert_count; 
    int total_plates_count;          
} StatisticsPlateCounts;

/**
 * @brief Medie giornaliere dei piatti.
 */
typedef struct {
    double average_daily_first_courses;
    double average_daily_second_courses;
    double average_daily_coffee_dessert;
    double average_daily_total;
} StatisticsPlateAverages;

/**
 * @brief Tempi di attesa medi espressi in minuti simulati.
 */
typedef struct {
    double average_wait_first_course;
    double average_wait_second_course;
    double average_wait_coffee_dessert;
    double average_wait_cash_desk;
    double average_wait_global;      
} StatisticsWaitTimes;

/**
 * @brief Statistiche relative ai flussi dei clienti (Utenti).
 */
typedef struct {
    int total_clients_served;
    int total_clients_not_served;
    double average_daily_clients_served;     
    double average_daily_clients_not_served; 
    int total_clients_with_ticket;       /**< Numero totale di utenti serviti con ticket */
    int total_clients_without_ticket;    /**< Numero totale di utenti serviti senza ticket */
} StatisticsClientData;

/**
 * @brief Performance e attività degli operatori della mensa.
 */
typedef struct {
    int total_active_simulations;    
    int daily_active_operators;        
    int total_breaks_taken;        
    double average_daily_breaks; 
} StatisticsOperatorData;

/**
 * @brief Analisi dei flussi finanziari della simulazione.
 */
typedef struct {
    double accumulated_total_income;
    double current_daily_income;
    double average_daily_income;
} StatisticsIncomeData;

/**
 * @brief Struttura aggregata delle Statistiche di Simulazione.
 * 
 * Contiene l'intero set di dati storici e giornalieri necessari
 * per il report finale e il monitoraggio in tempo reale.
 */
typedef struct {
    StatisticsPlateCounts total_served_plates;      
    StatisticsPlateCounts total_leftover_plates;   
    StatisticsPlateAverages average_daily_served_plates;   
    StatisticsPlateAverages average_daily_leftover_plates; 
    StatisticsWaitTimes total_average_wait_times;  
    StatisticsWaitTimes daily_average_wait_times;  
    
    StatisticsClientData clients_statistics;
    StatisticsOperatorData operators_statistics;
    StatisticsIncomeData income_statistics;

    TerminationReason reason_for_termination; /**< Causa della fine della simulazione */
} SimulationStatistics;

/**
 * @brief Legge e calcola le statistiche attuali dalla memoria condivisa.
 * 
 * @param shared_memory_ptr Puntatore alla memoria condivisa principale.
 * @return SimulationStatistics Struttura popolata con i dati estratti.
 */
SimulationStatistics collect_simulation_statistics(struct MainSharedMemory *shared_memory_ptr);

/**
 * @brief Visualizza a terminale un report dettagliato dei dati giornalieri.
 * 
 * @param statistics Struttura contenente i dati da stampare.
 * @param simulation_day Indice del giorno corrente (partendo da 0).
 */
void display_daily_statistics_report(SimulationStatistics statistics, int simulation_day);

#endif
