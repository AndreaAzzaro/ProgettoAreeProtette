#!/bin/bash
echo "Terminazione processi..."
pkill -9 -f responsabile_mensa || true
pkill -9 -f operatore || true
pkill -9 -f utente || true
pkill -9 -f operatore_cassa || true

echo "Pulizia risorse IPC..."
# Code di messaggi
ipcs -q | awk '/^[0-9]/ {print $2}' | while read id; do ipcrm -q "$id" && echo "Rimossa coda $id"; done
# Memoria condivisa
ipcs -m | awk '/^[0-9]/ {print $2}' | while read id; do ipcrm -m "$id" && echo "Rimossa SHM $id"; done
# Semafori
ipcs -s | awk '/^[0-9]/ {print $2}' | while read id; do ipcrm -s "$id" && echo "Rimasso semaforo $id"; done

echo "Cleanup completato."
