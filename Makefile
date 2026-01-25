# Makefile per Progetto Sistemi Operativi - System-V
# Progettato per essere modulare e scalabile

# Compilatore e Flag
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE -Iinclude
LDFLAGS = 

# Directory del progetto
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Moduli comuni (librerie interne)
COMMON_SRCS = $(SRC_DIR)/common/common.c \
              $(SRC_DIR)/config/menu.c \
              $(SRC_DIR)/config/config.c \
              $(SRC_DIR)/ipc/sem.c \
              $(SRC_DIR)/ipc/shm.c \
              $(SRC_DIR)/ipc/queue.c \
              $(SRC_DIR)/statistics/statistics.c \
              $(SRC_DIR)/utils/utils.c \
              $(SRC_DIR)/signal/signals_handler.c

COMMON_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(COMMON_SRCS))

# Programmi
PROGS = add_users communication_disorder operatore operatore_cassa responsabile_mensa utente
TARGETS = $(addprefix $(BIN_DIR)/, $(PROGS))

all: $(TARGETS)

# Regola per ogni eseguibile
$(BIN_DIR)/add_users: $(OBJ_DIR)/programs/add_users/add_users.o $(COMMON_OBJS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/communication_disorder: $(OBJ_DIR)/programs/communication_disorder/communication_disorder.o $(COMMON_OBJS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/operatore: $(OBJ_DIR)/programs/operatore/operatore.o $(COMMON_OBJS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/operatore_cassa: $(OBJ_DIR)/programs/operatore_cassa/operatore_cassa.o $(COMMON_OBJS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/responsabile_mensa: $(OBJ_DIR)/programs/responsabile_mensa/responsabile_mensa.o $(OBJ_DIR)/programs/responsabile_mensa/setup_ipc.o $(OBJ_DIR)/programs/responsabile_mensa/setup_population.o $(OBJ_DIR)/programs/responsabile_mensa/simulation_engine.o $(COMMON_OBJS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/utente: $(OBJ_DIR)/programs/utente/utente.o $(COMMON_OBJS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(CFLAGS) $^ -o $@

# Regola per compilare gli oggetti dei programmi
$(OBJ_DIR)/programs/%/%.o: $(SRC_DIR)/programs/%/%.c | $(OBJ_DIR)_dirs
	@echo "Compiling program module $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# Regola generale per compilare i moduli comuni
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)_dirs
	@echo "Compiling common module $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(OBJ_DIR)_dirs:
	@mkdir -p $(OBJ_DIR)/common $(OBJ_DIR)/config $(OBJ_DIR)/ipc $(OBJ_DIR)/statistics $(OBJ_DIR)/utils $(OBJ_DIR)/signal
	@mkdir -p $(addprefix $(OBJ_DIR)/programs/, $(PROGS))

clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Done."

.PHONY: all clean
