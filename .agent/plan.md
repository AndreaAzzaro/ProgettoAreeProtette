# Protocollo Operativo - Push to Main

## Obiettivo

Eseguire il commit e il push delle modifiche correnti sul ramo `main`.

## Stato Attuale

- File modificati: `src/programs/responsabile_mensa/simulation_engine.c` (aggiunto reset `add_users_flag`), `cleanup_ipc.sh`.
- File untracked: `.agent/plan.md`.

## Azioni

1. Eseguire `git add .` per includere tutte le modifiche.
2. Eseguire `git commit -m "Fix: reset add_users_flag in simulation_engine and update cleanup script"`
3. Eseguire `git push origin main`.

## Cosa NON toccherò

- Nessun file sorgente verrà ulteriormente modificato.
