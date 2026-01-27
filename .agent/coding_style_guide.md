# Linee Guida per la Documentazione del Codice C

Questo documento definisce le convenzioni di stile e documentazione per rendere il progetto chiaro, leggibile e facilmente valutabile.

---

## 1. Struttura dei File

### Header Files (`.h`)

Ogni header deve contenere:

1. **Doxygen File Block** - Descrizione del modulo
2. **Include Guards** - Prevenzione inclusioni multiple
3. **Includes** - Dipendenze ordinate (sistema, poi progetto)
4. **Costanti/Macro** - Con documentazione inline
5. **Typedef/Struct** - Con documentazione dei campi
6. **Prototipi Funzioni** - Con documentazione completa

```c
/**
 * @file nome_modulo.h
 * @brief Descrizione breve del modulo.
 *
 * Descrizione estesa che spiega lo scopo del modulo,
 * le sue responsabilità e come si integra nel sistema.
 */

#ifndef NOME_MODULO_H
#define NOME_MODULO_H

#include <stdio.h>      /* Includes di sistema */
#include "common.h"     /* Includes del progetto */

/** Costante con descrizione inline */
#define MAX_BUFFER_SIZE 1024

/**
 * @brief Descrizione breve della funzione.
 *
 * @param param1 Descrizione del primo parametro.
 * @param param2 Descrizione del secondo parametro.
 * @return Descrizione del valore di ritorno.
 */
int nome_funzione(int param1, char *param2);

#endif /* NOME_MODULO_H */
```

---

## 2. Source Files (`.c`)

### Struttura Consigliata

```c
/**
 * @file nome_modulo.c
 * @brief Implementazione del modulo [Nome].
 */

/* 1. INCLUDES */
#include <stdio.h>          /* Sistema */
#include "nome_modulo.h"    /* Header proprio */
#include "dipendenza.h"     /* Dipendenze progetto */

/* 2. MACRO LOCALI (se necessarie) */
#define HELPER_CONSTANT 42

/* 3. VARIABILI STATICHE (globali al file) */
static volatile sig_atomic_t flag_esempio = 0;

/* 4. PROTOTIPI FUNZIONI PRIVATE */
static void funzione_helper(int x);

/* 5. IMPLEMENTAZIONE FUNZIONI PUBBLICHE */

int nome_funzione(int param1, char *param2) {
    /* Implementazione */
}

/* 6. IMPLEMENTAZIONE FUNZIONI PRIVATE */

static void funzione_helper(int x) {
    /* Implementazione */
}
```

---

## 3. Stile dei Commenti nei File `.c`

### Commenti di Sezione

Usare blocchi visivi per separare sezioni logiche:

```c
/* ==========================================================================
 *                       SEZIONE: GESTIONE SEGNALI
 * ========================================================================== */
```

### Commenti di Funzione (stile semplificato)

Per funzioni già documentate nell'header, usare un commento breve:

```c
/**
 * Implementazione di nome_funzione.
 * @see nome_modulo.h per la documentazione completa.
 */
int nome_funzione(int param1, char *param2) {
    /* ... */
}
```

### Commenti Inline

Usare commenti `/* */` per spiegare il "perché", non il "cosa":

```c
/* BAD: Incrementa i */
i++;

/* GOOD: Compensa il decremento fatto in fase di cleanup */
i++;
```

### Commenti di Blocco Logico

Per sezioni complesse, numerare i passi:

```c
/* 1. Validazione input */
if (ptr == NULL) return -1;

/* 2. Acquisizione risorsa */
reserve_sem(sem_id, MUTEX_LOCK);

/* 3. Operazione critica */
shared_data->counter++;

/* 4. Rilascio risorsa */
release_sem(sem_id, MUTEX_LOCK);
```

---

## 4. Naming Conventions

| Elemento         | Convenzione            | Esempio                   |
| ---------------- | ---------------------- | ------------------------- |
| Funzioni         | `snake_case`           | `send_message_to_queue()` |
| Variabili locali | `snake_case`           | `user_count`              |
| Costanti/Macro   | `SCREAMING_SNAKE_CASE` | `MAX_USERS`               |
| Tipi (typedef)   | `PascalCase`           | `SimulationMessage`       |
| Enum Values      | `SCREAMING_SNAKE_CASE` | `BARRIER_MORNING_READY`   |
| File             | `snake_case.c/.h`      | `simulation_engine.c`     |

---

## 5. Formattazione

### Indentazione

- **4 spazi** (no tab)
- Massimo **100 caratteri** per riga

### Parentesi Graffe

Stile K&R (apertura sulla stessa riga):

```c
if (condizione) {
    /* codice */
} else {
    /* altro codice */
}
```

### Spaziatura

```c
/* Operatori con spazi */
x = a + b;
if (a == b) { }

/* Chiamate funzione senza spazio prima della parentesi */
funzione(arg1, arg2);

/* Virgole seguite da spazio */
int a, b, c;
```

---

## 6. Documentazione Speciale

### TODO/FIXME

```c
/* TODO: Implementare gestione timeout */
/* FIXME: Memory leak in caso di errore */
/* HACK: Workaround per bug kernel < 5.0 */
```

### Riferimenti a Specifiche

```c
/* [CONSEGNA 5.3] Implementazione pausa operatore */
/* [RFC 1234] Formato del messaggio */
```

### Punti di Sincronizzazione

```c
/* [BARRIER] Attesa tutti i processi pronti */
wait_for_zero(sem_id, BARRIER_READY);

/* [CRITICAL SECTION] Inizio zona protetta */
reserve_sem(mutex_id, MUTEX_DATA);
```

---

## 7. Checklist Pre-Consegna

- [ ] Tutti i file hanno il blocco Doxygen iniziale
- [ ] Nessuna funzione pubblica senza documentazione nell'header
- [ ] Commenti spiegano il "perché", non il "cosa"
- [ ] Nomi variabili autoesplicativi
- [ ] Magic numbers sostituiti con costanti nominate
- [ ] Nessun `printf` di debug rimasto
- [ ] Codice compila senza warning (`-Wall -Wextra`)
- [ ] Indentazione consistente in tutto il progetto

---

## 8. Comandi Utili

### Generare documentazione Doxygen

```bash
doxygen Doxyfile
```

### Verificare stile con `clang-format`

```bash
clang-format -i src/**/*.c include/**/*.h
```

### Contare linee di codice

```bash
find src include -name "*.c" -o -name "*.h" | xargs wc -l
```
