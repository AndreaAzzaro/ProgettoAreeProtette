/**
 * @file config.h
 * @brief Definizioni delle strutture dati per la configurazione della simulazione.
 */

#ifndef CONFIG_H
#define CONFIG_H

/**
 * @brief Parametri quantitativi delle entità nella simulazione.
 */
typedef struct {
    int number_of_workers;              /**< Numero totale di operatori attivi */
    int number_of_initial_users;        /**< Numero iniziale di utenti all'avvio */
    int number_of_new_users_batch;     /**< Numero di utenti aggiunti in ogni batch */
    int number_of_allowed_breaks;       /**< Numero massimo di pause consentite per operatore */
    int maximum_users_per_group;        /**< Dimensione massima di un gruppo di utenti */
} ConfigurationQuantities;

/**
 * @brief Capacità e posti disponibili nelle varie aree.
 */
typedef struct {
    int seats_first_course;            /**< Posti al bancone dei Primi Piatti */
    int seats_second_course;           /**< Posti al bancone dei Secondi Piatti */
    int seats_coffee_dessert;          /**< Posti al bancone Bar/Dolci */
    int seats_cash_desk;               /**< Postazioni attive in Cassa */
    int total_dining_seats;            /**< NOFTABLESEATS: Posti a sedere totali nell'area refezione */
} ConfigurationSeats;

/**
 * @brief Prezzi unitari per tipologia di piatto.
 */
typedef struct {
    double price_first_course;
    double price_second_course;
    double price_coffee_dessert;
} ConfigurationPrices;

/**
 * @brief Parametri temporali della simulazione.
 */
typedef struct {
    int simulation_duration_days;       /**< Numero di giorni totali della simulazione */
    int meal_duration_minutes;          /**< Durata fittizia di un pasto */
    long nanoseconds_per_tick;          /**< Fattore di scala temporale (Nanosleep) */
    int average_service_time_primi;
    int average_service_time_secondi;
    int average_service_time_coffee;
    int average_service_time_cassa;
    int average_service_time_ticket;    /**< Tempo medio per la lettura del ticket */
    int average_refill_time;
    int stop_duration_minutes;
} ConfigurationTimings;

/**
 * @brief Soglie operative e limiti di rifornimento.
 */
typedef struct {
    int overload_threshold;             /**< Soglia di carico per attivazione logiche di emergenza */
    int maximum_portions_primi;         /**< Capacità massima scorte Primi */
    int maximum_portions_secondi;       /**< Capacità massima scorte Secondi */
    int refill_amount_primi;            /**< Quantità aggiunta ad ogni refill Primi */
    int refill_amount_secondi;          /**< Quantità aggiunta ad ogni refill Secondi */
    int queue_patience_threshold;       /**< Tempo massimo di attesa prima che un utente abbandoni */
} ConfigurationThresholds;

/**
 * @brief Struttura Master di Configurazione.
 */
typedef struct {
    ConfigurationQuantities quantities;
    ConfigurationSeats seats;
    ConfigurationPrices prices;
    ConfigurationThresholds thresholds;
    ConfigurationTimings timings;
} SimulationConfiguration;

/**
 * @brief Carica la configurazione dal file di sistema.
 */
SimulationConfiguration load_simulation_configuration(); 

#endif