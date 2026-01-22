#ifndef MENU_H
#define MENU_H

#define MAX_DISH_NAME 32
#define MAX_DISHES_PER_TYPE 20
#define MENU_CONFIG_PATH "config/menu.txt"

// Enum per identificare il tipo internamente (utile per switch/logica)
typedef enum {
    DISH_PRIMO = 0,
    DISH_SECONDO,
    DISH_CONTORNO,
    DISH_DESSERT,
    DISH_BEVERAGE,
    DISH_TYPE_COUNT // Utility: vale 5
} DishType;

// Singola pietanza: Nome
typedef struct {
    char name[MAX_DISH_NAME];
} Dish;

// Struttura Menu (Pensata per SHM: no puntatori, solo array statici)
typedef struct {
    Dish primi[MAX_DISHES_PER_TYPE];
    int n_primi;

    Dish secondi[MAX_DISHES_PER_TYPE];
    int n_secondi;

    Dish contorni[MAX_DISHES_PER_TYPE];
    int n_contorni;

    Dish desserts[MAX_DISHES_PER_TYPE];
    int n_desserts;

    Dish beverages[MAX_DISHES_PER_TYPE]; // Caffè o Bevande
    int n_beverages;

} Menu;

/**
 * @brief Carica il menu da file di testo.
 * Legge menu.txt e riempie la struct Menu.
 * Termina il processo in caso di errore.
 */
Menu load_menu();

#endif
