# Makefile per Progetto Sistemi Operativi - System-V
# Progettato per essere modulare e scalabile

# Compilatore e Flag
CC = gcc
# -D_GNU_SOURCE è fondamentale per abilitare le estensioni GNU (riferimento consegna riga 273-274)
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE -Iinclude
LDFLAGS = 

# Directory del progetto
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INCLUDE_DIR = include

# Moduli comuni (librerie interne)
COMMON_SRCS = $(SRC_DIR)/common/common.c \
              $(SRC_DIR)/common/menu.c \
              $(SRC_DIR)/config/config.c \
              $(SRC_DIR)/ipc/sem.c \
              $(SRC_DIR)/ipc/shm.c \
              $(SRC_DIR)/ipc/queue.c \
              $(SRC_DIR)/statistics/statistics.c \
              $(SRC_DIR)/utils/utils.c

# Trasforma i .c in .o mantenendo la struttura delle cartelle in obj/
COMMON_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(COMMON_SRCS))

# Lista dei programmi da generare
PROGS = add_users communication_disorder operatore operatore_cassa responsabile_mensa utente
TARGETS = $(addprefix $(BIN_DIR)/, $(PROGS))

# Regola di default
all: $(TARGETS)

# Regola per linkare ogni singolo programma
# Ogni programma dipende dai suoi oggetti specifici e da tutti i moduli comuni
$(BIN_DIR)/%: $(OBJ_DIR)/programs/%/%.o $(COMMON_OBJS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Regola per compilare gli oggetti dei programmi (aggiunge il path locale agli include)
$(OBJ_DIR)/programs/%.o: $(SRC_DIR)/programs/%.c | $(OBJ_DIR)_dirs
	@echo "Compiling program module $<..."
	@$(CC) $(CFLAGS) -I$(dir $<) -c $< -o $@

# Regola generale per compilare i moduli comuni
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)_dirs
	@echo "Compiling common module $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# Creazione cartelle necessarie
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(OBJ_DIR)_dirs:
	@mkdir -p $(OBJ_DIR)/common $(OBJ_DIR)/config $(OBJ_DIR)/ipc $(OBJ_DIR)/statistics $(OBJ_DIR)/utils
	@mkdir -p $(addprefix $(OBJ_DIR)/programs/, $(PROGS))

# Pulizia dei file compilati
clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Done."

.PHONY: all clean
