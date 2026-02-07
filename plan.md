# Piano di Esportazione su GitHub

## Obiettivo

Creare una nuova repository su GitHub e caricare i file del progetto attuale.

## Fasi Operative

1. **Pulizia Filesystem:** Rimuovere i file duplicati `:Zone.Identifier` prodotti dal filesystem.
2. **Analisi Locale:** Verificare lo stato attuale di Git.
3. **Creazione Repository GitHub:**
   - Utilizzare il `browser_subagent` per creare una nuova repository chiamata `AzzaroDiNicolaAvaro` (o nome simile dedotto dal path).
4. **Push Finale:** Rimuovere il vecchio `origin`, aggiungere il nuovo e caricare i file.

## Cosa NON toccherò

- Non modificherò il codice sorgente del progetto.
- Non eliminerò file esistenti (a meno che non siano conflittuali con Git).
- Non caricherò file sensibili o binari se non strettamente necessario (verificherò `.gitignore`).

## Red Team Review

_Rischio:_ La mancanza di autenticazione GitHub CLI.
_Soluzione:_ Se `gh auth status` fallisce, fornirò i comandi manuali per legare una repo esistente creata via browser.
