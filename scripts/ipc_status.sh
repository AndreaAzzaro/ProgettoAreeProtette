#!/bin/bash
###############################################################################
# @file ipc_status.sh
# @brief Script per visualizzare lo stato delle risorse IPC della simulazione.
#
# Mostra in modo leggibile lo stato di tutti i semafori, code di messaggi
# e memoria condivisa utilizzando gli ID fissi definiti in include/ipc_keys.h
#
# Utile per debugging e monitoraggio dello stato della simulazione.
###############################################################################

echo "========================================="
echo "  IPC Status - Simulazione Mensa"
echo "========================================="

# Colori per output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Funzione per mostrare info su un semaforo
show_sem() {
    local key=$1
    local name=$2

    # Converti chiave decimale in esadecimale per ipcs
    local key_hex=$(printf "0x%08x" $key)

    # Trova l'ID reale usando la chiave
    local semid=$(ipcs -s | awk -v key="$key_hex" '$1 == key {print $2}')

    if [ ! -z "$semid" ]; then
        echo -e "${GREEN}✓${NC} ${BLUE}Chiave ${key} (ID ${semid})${NC} - ${name}"

        # Recupera il numero di semafori nel set
        nsems=$(ipcs -s -i ${semid} 2>/dev/null | grep "nsems" | awk '{print $3}')

        if [ ! -z "$nsems" ]; then
            echo "  └─ Set di ${nsems} semafori"

            # Mostra i valori di ogni semaforo (limitato ai primi 10 per evitare output troppo lungo)
            for i in $(seq 0 $((nsems < 10 ? nsems - 1 : 9))); do
                val=$(ipcs -s -i ${semid} 2>/dev/null | grep "semval" | sed -n "$((i+1))p" | awk '{print $3}')
                if [ ! -z "$val" ]; then
                    echo "     [$i] = $val"
                fi
            done

            if [ $nsems -gt 10 ]; then
                echo "     ... (altri $((nsems - 10)) semafori non mostrati)"
            fi
        fi
        echo ""
    else
        echo -e "${RED}✗${NC} Chiave ${key} - ${name} ${YELLOW}(non trovata)${NC}"
    fi
}

# Funzione per mostrare info su una coda
show_queue() {
    local key=$1
    local name=$2

    # Converti chiave decimale in esadecimale per ipcs
    local key_hex=$(printf "0x%08x" $key)

    # Trova l'ID reale usando la chiave
    local msqid=$(ipcs -q | awk -v key="$key_hex" '$1 == key {print $2}')

    if [ ! -z "$msqid" ]; then
        echo -e "${GREEN}✓${NC} ${CYAN}Chiave ${key} (ID ${msqid})${NC} - ${name}"

        # Recupera numero di messaggi e bytes totali
        qnum=$(ipcs -q -i ${msqid} 2>/dev/null | grep "messages" | awk '{print $5}')
        qbytes=$(ipcs -q -i ${msqid} 2>/dev/null | grep "used-bytes" | awk '{print $5}')

        if [ ! -z "$qnum" ] && [ ! -z "$qbytes" ]; then
            echo "  └─ Messaggi: ${qnum}, Bytes usati: ${qbytes}"
        fi
        echo ""
    else
        echo -e "${RED}✗${NC} Chiave ${key} - ${name} ${YELLOW}(non trovata)${NC}"
    fi
}

# Funzione per mostrare info su memoria condivisa
show_shm() {
    local key=$1
    local name=$2

    # Converti chiave decimale in esadecimale per ipcs
    local key_hex=$(printf "0x%08x" $key)

    # Trova l'ID reale usando la chiave
    local shmid=$(ipcs -m | awk -v key="$key_hex" '$1 == key {print $2}')

    if [ ! -z "$shmid" ]; then
        echo -e "${GREEN}✓${NC} ${CYAN}Chiave ${key} (ID ${shmid})${NC} - ${name}"

        # Recupera dimensione e numero di attach
        segsz=$(ipcs -m -i ${shmid} 2>/dev/null | grep "bytes" | head -1 | awk '{print $5}')
        nattch=$(ipcs -m -i ${shmid} 2>/dev/null | grep "nattch" | awk '{print $3}')

        if [ ! -z "$segsz" ] && [ ! -z "$nattch" ]; then
            # Converti bytes in formato leggibile
            if [ $segsz -gt 1048576 ]; then
                size_mb=$(echo "scale=2; $segsz / 1048576" | bc)
                echo "  └─ Dimensione: ${size_mb} MB, Processi attach: ${nattch}"
            elif [ $segsz -gt 1024 ]; then
                size_kb=$(echo "scale=2; $segsz / 1024" | bc)
                echo "  └─ Dimensione: ${size_kb} KB, Processi attach: ${nattch}"
            else
                echo "  └─ Dimensione: ${segsz} bytes, Processi attach: ${nattch}"
            fi
        fi
        echo ""
    else
        echo -e "${RED}✗${NC} Chiave ${key} - ${name} ${YELLOW}(non trovata)${NC}"
    fi
}

echo ""
echo "SEMAFORI"
echo "========================================"
show_sem 1000 "Barriere di sincronizzazione (SYNC_BARRIER)"
show_sem 1100 "Mutex globali (MUTEX)"
show_sem 1200 "Pool sincronizzazione gruppi"
show_sem 1300 "Validatori ticket (4 slot)"
show_sem 1400 "Posti a sedere dining area"
show_sem 1500 "Stazione primi piatti"
show_sem 1600 "Stazione secondi piatti"
show_sem 1700 "Stazione caffè/dolci"
show_sem 1800 "Stazione cassa"

echo ""
echo "CODE DI MESSAGGI"
echo "========================================"
show_queue 2000 "Ordini primi piatti"
show_queue 2100 "Ordini secondi piatti"
show_queue 2200 "Ordini caffè/dolci"
show_queue 2300 "Pagamenti cassa"
show_queue 2400 "Controllo add_users"

echo ""
echo "MEMORIA CONDIVISA"
echo "========================================"
show_shm 3000 "Memoria principale simulazione"

echo ""
echo "========================================="
echo "Per rimuovere tutte le risorse IPC:"
echo "  ./scripts/ipc_cleanup.sh"
echo "========================================="
