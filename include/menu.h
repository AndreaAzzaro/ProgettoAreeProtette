/**
 * @file menu.h
 * @brief Definizioni per la gestione del menu della mensa.
 */

#ifndef MENU_H
#define MENU_H

#define MAX_DISH_NAME_LENGTH 32
#define MAX_DISHES_PER_CATEGORY 20
#define MENU_CONFIGURATION_PATH "config/menu.conf"

/**
 * @brief Identificatori per le categorie di piatti nel menu.
 */
typedef enum {
    MENU_DISH_TYPE_FIRST_COURSE = 0,
    MENU_DISH_TYPE_SECOND_COURSE = 1,
    MENU_DISH_TYPE_SIDE_COURSE = 2,
    MENU_DISH_TYPE_DESSERT = 3,
    MENU_DISH_TYPE_BEVERAGE = 4,
    MENU_DISH_TYPE_COUNT
} MenuDishCategory;

typedef struct {
    char name[MAX_DISH_NAME_LENGTH];
} MenuDish;

typedef struct {
    MenuDish first_courses[MAX_DISHES_PER_CATEGORY];
    int number_of_first_courses;

    MenuDish second_courses[MAX_DISHES_PER_CATEGORY];
    int number_of_second_courses;

    MenuDish side_courses[MAX_DISHES_PER_CATEGORY];
    int number_of_side_courses;

    MenuDish dessert_courses[MAX_DISHES_PER_CATEGORY];
    int number_of_dessert_courses;

    MenuDish beverage_courses[MAX_DISHES_PER_CATEGORY];
    int number_of_beverage_courses;
} SimulationMenu;

/**
 * @brief Carica il menu da file.
 */
SimulationMenu load_simulation_menu();

/**
 * @brief Recupera il nome di un piatto partendo dalla categoria e dall'indice.
 * 
 * Questa funzione permette di risolvere l'ID scambiato nelle code di messaggi.
 * 
 * @param menu_ptr Puntatore alla struttura menu in memoria condivisa.
 * @param category Categoria del piatto (MenuDishCategory).
 * @param dish_index Indice del piatto all'interno della categoria.
 * @return const char* Nome del piatto o "Sconosciuto" in caso di errore.
 */
const char* get_dish_name_by_id(SimulationMenu *menu_ptr, MenuDishCategory category, int dish_index);

#endif
