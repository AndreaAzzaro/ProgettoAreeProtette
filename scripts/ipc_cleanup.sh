#!/bin/bash
###############################################################################
# @file ipc_cleanup.sh
# @brief Script per la pulizia completa delle risorse IPC della simulazione.
#
# Rimuove tutti i semafori, code di messaggi e memoria condivisa utilizzando
# gli ID fissi definiti in include/ipc_keys.h
#
# NOTA: Esegui questo script tra una esecuzione e l'altra della simulazione
# per garantire un ambiente pulito.
###############################################################################

echo "========================================="
echo "  IPC Cleanup - Simulazione Mensa"
echo "========================================="

# Colori per output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

removed_count=0
not_found_count=0

# Funzione per rimuovere un semaforo
remove_sem() {
    local key=$1
    local name=$2

    # Converti chiave decimale in esadecimale
    local key_hex=$(printf "0x%08x" $key)

    # Trova l'ID reale usando la chiave
    local semid=$(ipcs -s | awk -v key="$key_hex" '$1 == key {print $2}')

    if [ ! -z "$semid" ]; then
        if ipcrm -s ${semid} 2>/dev/null; then
            echo -e "${GREEN}✓${NC} Chiave ${key} (ID ${semid}) rimossa (${name})"
            ((removed_count++))
        else
            echo -e "${RED}✗${NC} Errore rimozione chiave ${key} (ID ${semid}) (${name})"
        fi
    else
        echo -e "${YELLOW}○${NC} Chiave ${key} non trovata (${name})"
        ((not_found_count++))
    fi
}

# Funzione per rimuovere una coda
remove_queue() {
    local key=$1
    local name=$2

    # Converti chiave decimale in esadecimale
    local key_hex=$(printf "0x%08x" $key)

    # Trova l'ID reale usando la chiave
    local msqid=$(ipcs -q | awk -v key="$key_hex" '$1 == key {print $2}')

    if [ ! -z "$msqid" ]; then
        if ipcrm -q ${msqid} 2>/dev/null; then
            echo -e "${GREEN}✓${NC} Chiave ${key} (ID ${msqid}) rimossa (${name})"
            ((removed_count++))
        else
            echo -e "${RED}✗${NC} Errore rimozione chiave ${key} (ID ${msqid}) (${name})"
        fi
    else
        echo -e "${YELLOW}○${NC} Chiave ${key} non trovata (${name})"
        ((not_found_count++))
    fi
}

# Funzione per rimuovere memoria condivisa
remove_shm() {
    local key=$1
    local name=$2

    # Converti chiave decimale in esadecimale
    local key_hex=$(printf "0x%08x" $key)

    # Trova l'ID reale usando la chiave
    local shmid=$(ipcs -m | awk -v key="$key_hex" '$1 == key {print $2}')

    if [ ! -z "$shmid" ]; then
        if ipcrm -m ${shmid} 2>/dev/null; then
            echo -e "${GREEN}✓${NC} Chiave ${key} (ID ${shmid}) rimossa (${name})"
            ((removed_count++))
        else
            echo -e "${RED}✗${NC} Errore rimozione chiave ${key} (ID ${shmid}) (${name})"
        fi
    else
        echo -e "${YELLOW}○${NC} Chiave ${key} non trovata (${name})"
        ((not_found_count++))
    fi
}

echo ""
echo "Rimozione SEMAFORI..."
echo "---------------------"
remove_sem 1000 "Barriere di sincronizzazione"
remove_sem 1100 "Mutex globali"
remove_sem 1200 "Pool gruppi"
remove_sem 1300 "Validatori ticket"
remove_sem 1400 "Posti a sedere"
remove_sem 1500 "Stazione primi piatti"
remove_sem 1600 "Stazione secondi piatti"
remove_sem 1700 "Stazione caffè/dolci"
remove_sem 1800 "Stazione cassa"

echo ""
echo "Rimozione CODE DI MESSAGGI..."
echo "------------------------------"
remove_queue 2000 "Coda primi piatti"
remove_queue 2100 "Coda secondi piatti"
remove_queue 2200 "Coda caffè/dolci"
remove_queue 2300 "Coda cassa"
remove_queue 2400 "Coda controllo"

echo ""
echo "Rimozione MEMORIA CONDIVISA..."
echo "-------------------------------"
remove_shm 3000 "Memoria principale"

echo ""
echo "========================================="
echo -e "${GREEN}Risorse rimosse:${NC} ${removed_count}"
echo -e "${YELLOW}Risorse non trovate:${NC} ${not_found_count}"
echo "========================================="

if [ $removed_count -gt 0 ]; then
    echo -e "${GREEN}Cleanup completato con successo!${NC}"
else
    echo -e "${YELLOW}Nessuna risorsa IPC da pulire.${NC}"
fi
