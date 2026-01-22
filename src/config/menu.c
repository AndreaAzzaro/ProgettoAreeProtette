#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/menu.h"

Menu load_menu() {
    Menu m;
    // Pulisce tutto a 0 per sicurezza
    memset(&m, 0, sizeof(Menu));

    FILE *file = fopen(MENU_CONFIG_PATH, "r");
    if (file == NULL) {
        // Fallback locale
        file = fopen("menu.txt", "r");
        if (file == NULL) {
            perror("ERRORE CRITICO: Impossibile aprire menu.txt");
            exit(EXIT_FAILURE);
        }
    }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        // Salta commenti e righe vuote
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        char type;
        char name[MAX_DISH_NAME];

        // Formato atteso: TIPO NOME (es: P Pasta_al_Pomodoro)
        if (sscanf(line, " %c %31s", &type, name) == 2) {
            
            switch (type) {
                case 'P': // Primo
                    if (m.n_primi < MAX_DISHES_PER_TYPE) {
                        strcpy(m.primi[m.n_primi].name, name);
                        m.n_primi++;
                    }
                    break;
                case 'S': // Secondo
                    if (m.n_secondi < MAX_DISHES_PER_TYPE) {
                        strcpy(m.secondi[m.n_secondi].name, name);
                        m.n_secondi++;
                    }
                    break;
                case 'C': // Contorno
                    if (m.n_contorni < MAX_DISHES_PER_TYPE) {
                        strcpy(m.contorni[m.n_contorni].name, name);
                        m.n_contorni++;
                    }
                    break;
                case 'D': // Dolce
                    if (m.n_desserts < MAX_DISHES_PER_TYPE) {
                        strcpy(m.desserts[m.n_desserts].name, name);
                        m.n_desserts++;
                    }
                    break;
                case 'B': // Bevanda/Caffè
                    if (m.n_beverages < MAX_DISHES_PER_TYPE) {
                        strcpy(m.beverages[m.n_beverages].name, name);
                        m.n_beverages++;
                    }
                    break;
                default:
                    fprintf(stderr, "Warning: Tipo piatto sconosciuto '%c' in menu.txt\n", type);
            }
        }
    }

    fclose(file);
    printf("Menu caricato: %d Primi, %d Secondi, %d Contorni, %d Dolci, %d Bevande.\n",
           m.n_primi, m.n_secondi, m.n_contorni, m.n_desserts, m.n_beverages);
    
    return m;
}
