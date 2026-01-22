#include "../../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_PATH "config/config.conf"
#define MAX_LINE_LEN 256

// 1. DEFINIAMO UN ENUM PER IDENTIFICARE LE CHIAVI
// Questo trasforma le stringhe in numeri gestibili dallo switch
typedef enum {
    KEY_UNKNOWN = 0,
    
    // Quantities
    KEY_N_WORKERS, KEY_N_USERS, KEY_N_NEW_USERS, KEY_N_PAUSE,
    
    // Seats
    KEY_SEATS_PRIMI, KEY_SEATS_SECONDI, KEY_SEATS_COFFEE, KEY_SEATS_CASSA, KEY_TOTAL_SEATS,
    
    // Prices
    KEY_PRICE_PRIMI, KEY_PRICE_SECONDI, KEY_PRICE_COFFEE,
    
    // Timings
    KEY_SIM_DURATION, KEY_N_NANO_SECS, 
    KEY_AVG_SERVICE_PRIMI, KEY_AVG_SERVICE_MAIN, KEY_AVG_SERVICE_COFFEE, 
    KEY_AVG_SERVICE_CASSA, KEY_AVG_REFILL_TIME, KEY_STOP_DURATION,
    
    // Threshold
    KEY_OVERLOAD_THRESHOLD
} ConfigKey;

// 2. CREIAMO UNA TABELLA DI MAPPING (LOOKUP TABLE)
// Associa la stringa nel file al valore Enum corrispondente
// 2. CREIAMO UNA TABELLA DI MAPPING (LOOKUP TABLE)
// Associa la stringa nel file al valore Enum corrispondente
typedef struct {
    const char *string_key;
    ConfigKey enum_key;
} KeyMap;

static const KeyMap mapping_table[] = {
    // Quantities (NOF_...)
    {"NOF_WORKERS", KEY_N_WORKERS},
    {"NOF_USERS", KEY_N_USERS},
    {"N_NEW_USERS", KEY_N_NEW_USERS}, // Questo è ok da config.txt
    {"NOF_PAUSE", KEY_N_PAUSE},
    
    // Seats (NOF_WK_SEATS_...)
    {"NOF_WK_SEATS_PRIMI", KEY_SEATS_PRIMI},
    {"NOF_WK_SEATS_SECONDI", KEY_SEATS_SECONDI},
    {"NOF_WK_SEATS_COFFEE", KEY_SEATS_COFFEE},
    {"NOF_WK_SEATS_CASSA", KEY_SEATS_CASSA},
    {"NOF_TABLE_SEATS", KEY_TOTAL_SEATS}, // O TOTAL_SEATS, dipenda da config.txt. Useremo TOTAL_SEATS come fallback
    
    // Prices (PRICE_...)
    {"PRICE_PRIMI", KEY_PRICE_PRIMI},
    {"PRICE_SECONDI", KEY_PRICE_SECONDI},
    {"PRICE_COFFEE", KEY_PRICE_COFFEE},
    
    // Timings
    {"SIM_DURATION", KEY_SIM_DURATION},
    {"NNANOSECS", KEY_N_NANO_SECS},
    {"AVG_SRVC_PRIMI", KEY_AVG_SERVICE_PRIMI},
    {"AVG_SRVC_SECONDI", KEY_AVG_SERVICE_MAIN}, // Mappato su SECONDI nel txt
    {"AVG_SRVC_COFFEE", KEY_AVG_SERVICE_COFFEE},
    {"AVG_SRVC_CASSA", KEY_AVG_SERVICE_CASSA},
    {"AVG_REFILL_TIME", KEY_AVG_REFILL_TIME},
    {"STOP_DURATION", KEY_STOP_DURATION},
    
    // Threshold
    {"OVERLOAD_THRESHOLD", KEY_OVERLOAD_THRESHOLD},
    
    {NULL, KEY_UNKNOWN} // Sentinella di fine array
};

// 3. FUNZIONE DI RISOLUZIONE
// Converte stringa -> Enum scorrendo la tabella
ConfigKey resolve_key(const char *key) {
    for (int i = 0; mapping_table[i].string_key != NULL; i++) {
        if (strcmp(key, mapping_table[i].string_key) == 0) {
            return mapping_table[i].enum_key;
        }
    }
    return KEY_UNKNOWN;
}

// 4. IMPLEMENTAZIONE LOAD CONFIG CON SWITCH
SimConfig loadConfig() {
    SimConfig cfg;
    memset(&cfg, 0, sizeof(SimConfig));

    FILE *file = fopen(CONFIG_PATH, "r");
    if (file == NULL) {
        // Fallback: prova se esiste nella directory corrente
        file = fopen("config.txt", "r");
        if (file == NULL) {
            perror("ERRORE CRITICO: Impossibile aprire il file di configurazione");
            exit(EXIT_FAILURE);
        }
    }

    char line[MAX_LINE_LEN];
    char key_str[100];
    long val;

    while (fgets(line, sizeof(line), file)) {
        // Salta commenti e righe vuote
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        // Parsing robusto: cerca CHIAVE=VALORE o CHIAVE = VALORE
        char *eq_pos = strchr(line, '=');
        if (eq_pos) {
            *eq_pos = '\0'; // Spezza la stringa sull'uguale
            char *key = line;
            char *val_str = eq_pos + 1;

            // Trim manuale (molto grezzo ma efficace per questo scopo)
            while(*key == ' ') key++; // Salta spazi iniziali chiave
            // Per la chiave, dobbiamo rimuovere spazi finali PRIMA dell'uguale
            int len = strlen(key);
            while(len > 0 && (key[len-1] == ' ' || key[len-1] == '\t')) key[--len] = '\0';

            val = atol(val_str); // Converte il valore
            strcpy(key_str, key); // Copia per lo switch

            // --- QUI AVVIENE LA MAGIA ---
            switch (resolve_key(key_str)) {
                
                // Quantities
                case KEY_N_WORKERS:     cfg.quantities.n_workers = (int)val; break;
                
                /* Mapping corretto per i nomi usati in config.txt */
                case KEY_N_USERS:       cfg.quantities.n_users = (int)val; break;
                case KEY_N_NEW_USERS:   cfg.quantities.n_new_users = (int)val; break;
                case KEY_N_PAUSE:       cfg.quantities.n_pause = (int)val; break;
                
                // Seats
                case KEY_SEATS_PRIMI:   cfg.seat.primi = (int)val; break;
                case KEY_SEATS_SECONDI: cfg.seat.secondi = (int)val; break;
                case KEY_SEATS_COFFEE:  cfg.seat.coffee = (int)val; break;
                case KEY_SEATS_CASSA:   cfg.seat.cassa = (int)val; break;
                case KEY_TOTAL_SEATS:   cfg.seat.seats = (int)val; break;
                
                // Prices
                case KEY_PRICE_PRIMI:   cfg.price.primi = (int)val; break;
                case KEY_PRICE_SECONDI: cfg.price.secondi = (int)val; break;
                case KEY_PRICE_COFFEE:  cfg.price.coffee = (int)val; break;
                
                // Timings
                case KEY_SIM_DURATION:       cfg.timing.sim_duration = (int)val; break;
                case KEY_N_NANO_SECS:        cfg.timing.n_nano_secs = val; break;
                case KEY_AVG_SERVICE_PRIMI:  cfg.timing.avg_service_primi = (int)val; break;
                case KEY_AVG_SERVICE_MAIN:   cfg.timing.avg_service_main = (int)val; break;
                case KEY_AVG_SERVICE_COFFEE: cfg.timing.avg_service_coffee = (int)val; break;
                case KEY_AVG_SERVICE_CASSA:  cfg.timing.avg_service_cassa = (int)val; break;
                case KEY_AVG_REFILL_TIME:    cfg.timing.avg_refill_time = (int)val; break;
                case KEY_STOP_DURATION:      cfg.timing.stop_duration = (int)val; break;
                
                // Threshold
                case KEY_OVERLOAD_THRESHOLD: cfg.threshold.overload_threshold = (int)val; break;

                case KEY_UNKNOWN:
                default:
                    // printf("Warning: Chiave ignorata: '%s'\n", key_str); // Debug verbose off
                    break;
            }
        }
    }

    fclose(file);
    printf("Configurazione caricata correttamente.\n");
    return cfg;
}