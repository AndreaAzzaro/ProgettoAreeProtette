## Servizio Mensa ‘Oasi del Golfo’

### Botta, Radicioni, Schifanella C.

         - Progetto SO 2025/
      - Versione non definitiva del 25 novembre

- 1 Composizione gruppo di studenti Indice
- 2 Consegna
- 3 Valutazione e validit`a
- 4 Pubblicazione del proprio progetto, plagio, sanzioni
- 5 Descrizione del progetto: versione minima (voto max 24 su 30)
  - 5.1 Processo responsabilemensa
  - 5.2 Risorse di tipo stazione
  - 5.3 Processo operatore
  - 5.4 Processo operatorecassa
  - 5.5 Processo utente
  - 5.6 Terminazione
- 6 Descrizione del progetto: versione “completa” (max 30)
- 7 Configurazione
- 8 Requisiti implementativi

## 1 Composizione gruppo di studenti Indice

Il progetto sar`a sviluppato da un gruppo, composto da 1 a massimo 3 componenti. Il gruppo dovrebbe essere
composto da studenti dello stesso turno, i quali discuteranno con il docente del proprio turno.E tuttavia consentita`
anche la discussione del progetto di laboratorio da parte di un gruppo di studenti di turni diversi. In questo caso,
tutti gli studenti del gruppo discuteranno con lo stesso docente. Esempio: Tizio (turno T1) e Caio (turno T2)
decidono di fare il progetto insieme. Lo consegnano e vengono convocati dal prof. Sempronio il giorno X. Tale
giorno Tizio e Caio si presentano muniti di un computer su cui il progetto `e sicuramente funzionante e ricevono
entrambi una valutazione dal Prof. Sempronio (docente del T1, anche se Caio fa parte del turno T2, il cui docente
di riferimento `e il prof. Calpurnio).

## 2 Consegna

Il progetto `e costituito da:

1. il codice sorgente
2. una breve relazione che sintetizzi le scelte progettuali compiute

Il progetto si consegna compilando la seguente Google Form, cui si accede con credenziali istituzionali,

- https://forms.gle/DMvHpL87ju4heYGA

la quale richieder`a il caricamento di:

- progetto stesso (un unico file in formato .tgz o .zip NON .rar). Il nome del file deve essere composto dall’unione
  dei cognomi dei componenti del gruppo (es.: TizioCaio.zip)
- cognome, nome, matricola, email di ciascun componente del gruppo.

Dopo il caricamento del progetto, verrete convocati dal docente con cui discuterete (si veda Sezione 1 in caso di
gruppo composto da studenti di turni diversi). Attenzione: compilare una sola form per progetto (e non una
per ogni componente del gruppo). Una eventuale ulteriore consegna prima dell’appuntamento annuller`a la
data dell’appuntamento.
La consegna deve avvenire almeno 10 giorni prima degli appelli scritti per dare modo al docente di pianificare
i colloqui:

- se consegnate con almeno 10 giorni di anticipo rispetto alla data di un appello, il docente propone una data
  per la discussione entro l’appello seguente
- altrimenti, la data sar`a successiva all’appello seguente.

Esempio: per avere la certezza di un appuntamento per la discussione di progetto entro l’appello scritto del
27/01/2026, si deve consegnare entro le ore 24:00 del 17/01/2026.

## 3 Valutazione e validit`a

Il progetto descritto nel presente documento potr`a essere discusso se consegnato entro il 30 Novembre 2026.
Da Dicembre 2026 sar`a discusso il progetto assegnato durante l’anno accademico 2026/27.
Tutti i membri del gruppo devono partecipare alla discussione. La valutazione del progetto `e individuale ed
espressa in 30-esimi. Durante la discussione

- verr`a chiesto di illustrare il progetto;
- verr`a chiesto di commentare il codice e eventualmente di apportare piccole modifiche al progetto;

- verranno proposti quesiti sul programma “Unix” del corso anche non necessariamente legati allo sviluppo del
  progetto.

E necessario ottenere una votazione di almeno` 18 su 30 per poter essere ammessi allo scritto. In caso di superamento
della discussione del progetto, la votazione conseguita consentir`a di partecipare allo scritto per i cinque appelli
successivi alla data di superamento. Non sono ammesse eccezioni o deroghe a tale periodo di validit`a. In caso
di mancato superamento, lo studente si potr`a ripresentare soltanto dopo almeno un mese dalla data del mancato
superamento
Il voto del progetto ha un peso di 1/3 sulla votazione finale dell’insegnamento di Sistemi Operativi.

## 4 Pubblicazione del proprio progetto, plagio, sanzioni

Copiare altri progetti o parte di essi impedisce una corretta valutazione. Per questo motivo, gli studenti che
consegnano il progetto sono consapevoli che:

- la condivisione con altri gruppi attraverso canali pubblici o privati (a titolo di esempio: google drive, canali
  telegram, github, etc.) di tutto o parte del progetto non `e consentita fino a tutto Novembre 2026;
- la copiatura di tutto o parte del progetto non `e consentita;
- eventuali frammenti di codice estratti da parti di codice visto a lezione o da altri repository pubblici devono
  essere opportunamente dichiarati nei commenti del codice e i candidati devono essere in grado di illustrarne
  il funzionamento.

Nel momento in cui uno studente non rispettasse le sopra citate norme di comportamento, dopo essere stato
convocato ed aver avuto modo di illustrare la propria posizione, potr`a essere oggetto delle seguenti sanzioni:

- se lo studente avr`a nel frattempo superato l’esame di Sistemi Operativi anche successivamente alla data di
discussione del progetto, la verbalizzazione del voto verr`a annullata;
- se lo studente avr`a soltanto superato la discussione del progetto ma non l’esame, la valutazione del progetto
verr`a annullata e lo studente non potr`a accedere ad ulteriore discussione di progetto prima dei due appelli
  successivi alla data di evidenza di copiatura.

## 5 Descrizione del progetto: versione minima (voto max 24 su 30)

Si intende simulare il funzionamento di una mensa studentesca/aziendale. A tal fine sono presenti i seguenti
processi e risorse: cassa di pagamento, stazioni di distribuzione cibo, tavoli, bancone del caff`e, cassiere, operatori e
responsabile mensa, studente/utente.

- Processo responsabilemensa che gestisce la simulazione, e mantiene le statistiche su richieste e servizi erogati
  dalla mensa. Genera tutti gli altri processi del simulatore.
- Processo operatorestazioneprimi: eroga i primi piatti. In particolare, dovranno essere implementate le
  funzionalit`a per garantire la scelta tra almeno 2 primi piatti ogni giorno. Il tempo medio per erogare il servizio
`e AVGSRVCPRIMI secondi, e dovr`a essere usato per generare un tempo casuale di erogazione nell’intorno±50%
  del valore indicato.
- Processo operatorestazionesecondi: eroga i secondi piatti e i contorni. In particolare, dovranno essere
  implementate le funzionalit`a per garantire almeno la scelta tra 2 secondi piatti con relativi contorni ogni
giorno. Il tempo medio per erogare il servizio `e AVGSRVCMAINCOURSE secondi, e dovr`a essere usato per
  generare un tempo casuale di erogazione nell’intorno±50% del valore indicato.
- Processo operatorestazionecoffee: eroga i caff`e e il dolce. In particolare, dovranno essere implementate
le funzionalit`a per garantire almeno la scelta tra 4 tipi di caff`e (normale, macchiato, decaffeinato, ginseng). Il
tempo medio per erogare il servizio `e AVGSRVCCOFFEE secondi, e dovr`a essere usato per generare un tempo
  casuale di erogazione nell’intorno±80% del valore indicato.

- Processo operatorecassa (cassiere): gestisce i pagamenti degli utenti. Il tempo medio per erogare il ser-
  vizio `e AVGSRVCCASSA secondi, e dovr`a essere usato per generare un tempo casuale di erogazione nell’intorno
  ±20% del valore indicato.
- Esistono NOFWORKERS processi di tipo operatore: hanno un orario di lavoro, ed effettuano pause casuali.
- Processi di tipo utente. In base alla situazione delle code alle varie stazioni di erogazione cibo, l’utente
  decide in quale coda inserirsi; ottenuto il piatto, decide se procedere alla cassa o passare a un’altra stazione di
  distribuzione cibo, e poi recarsi alla cassa. Una volta pagati alla cassa i piatti ottenuti, mangia per un tempo
  proporzionale al numero di piatti acquistati, e poi torna a casa/ufficio/aula. - Prima di iniziare a consumare i pasti, gli utenti devono passare a pagare presso la stazionecassa.
- Esistono NOFUSERS processi di tipo utente. Il processo utente decide se recarsi in mensa e sceglie cosa
  mangiare (si veda la descrizione del processo nella sezione 5.5).
- Esistono risorse di tipo stazionedidistribuzionecibo e stazionidirefezione (o, meno pomposa-
  mente, tavoli); ogni stazione `e destinata a fornire un solo tipo di cibo (primo/secondo-contorno/dolce-caff`e),
  oppure a consentire agli utenti di consumare il cibo acquistato. Esiste inoltre un altro tipo di stazione, la
  stazionecassa.
- Ogni stazione di distribuzione `e dotata di un certo numero di postazioni: NOFWKSEATSPRIMI, NOFWKSEATSSE-
  CONDI, NOFWKSEATSCOFFEE, NOFWKSEATSCASSA.

### 5.1 Processo responsabilemensa

Il processo responsabilemensa `e responsabile dell’avvio della simulazione, della creazione delle risorse di tipo
stazionedistribuzione e stazionerefezione, dei processi operatore e utente, delle statistiche e della termi-
nazione. Si noti bene che il processo responsabilemensa non si occupa dell’aggiornamento delle statistiche, ma
solo della loro lettura, secondo quanto indicato. All’avvio, il processo responsabilemensa:

- crea NOFWORKERS processi di tipo operatore e operatorecassa, assegnandoli alle stazionididistribuzione
  o alle casse, garantendo che nessuna stazione rimanga senza operatore.
- crea NOFUSERS processi di tipo utente.
- crea le 4 stazionidistribuzione per primi, secondi, caff`e e la cassa.
- crea NOFWKSEATSPRIMI, NOFWKSEATSSECONDI, NOFWKSEATSCOFFEE, NOFWKSEATSCASSA risorse di tipo
  postazione, per la corrispondente stazionedidistribuzione.
- crea NOFTABLESEATS risorse di tipo stazionerefezione.

Successivamente il responsabilemensa avvia la simulazione, che avr`a come durata SIMDURATION giorni, dove
ciascun minuto `e simulato dal trascorrere di NNANOSECS nanosecondi.
La simulazione deve cominciare solamente quando tutti i processi cassiere, operatore e utente sono stati creati
e hanno terminato la fase di inizializzazione.
Alla fine di ogni giornata, il processo responsabilemensa dovr`a stampare le statistiche totali e quelle della
giornata, che comprendono:

- il numero di utenti serviti totali nella simulazione;
- il numero di utenti non serviti totali nella simulazione;
- il numero di utenti serviti in media al giorno;
- il numero di utenti non serviti in media al giorno;
- il numero di piatti distribuiti totali e per tipo (primo, secondo-contorno, dolce-caff`e) nella simulazione;

- il numero di piatti avanzati totali e per tipo (primo, secondo-contorno, dolce-caff`e) nella simulazione;
- il numero di piatti distribuiti in media (totali e per tipo) al giorno;
- il numero di piatti avanzati in media (totali e per tipo) al giorno;
- il tempo medio (complessivo e su ciascuna stazione) di attesa degli utenti nella simulazione;
- il tempo medio di attesa (complessivo e su ciascuna stazione) degli utenti nella giornata;
- il numero di operatori attivi durante la giornata. Un operatore si considera attivo se `e riuscito ad accedere
  ad una postazione;
- il numero di operatori attivi durante la simulazione;
- il numero medio di pause effettuate nella giornata e il totale di pause effettuate durante la simulazione;
- il ricavo giornaliero;
- il ricavo totale su tutta la simulazione e medio per ciascuna giornata (a fine simulazione).

### 5.2 Risorse di tipo stazione

All’inizio della giornata, gli operatori sono associati a una stazione (di tipo distribuzione o cassa) dal responsabi-
lemensa che assegna sempre almeno un operatore a ciascuna stazione, e poi utilizza come criterio preferenziale
quello di assegnare pi`u operatori a mansioni il cui tempo medio di svolgimento (AVGSRVC∗) `e pi`u alto. La politica
di associazione operatore-servizio `e definita dal progettista, e deve essere applicata all’inizio di ogni giornata.
Le stazioni di refezione dei primi e dei secondi vengono inizializzate –all’inizio di ogni giorno– con un numer
pari a AVGREFILLPRIMI e AVGREFILLSECONDI di porzioni per ciascun tipo di piatto; e poi alimentate, ogni 10
minuti, fino ad un massimo numero di porzioni MAXPORZIONIPRIMI e MAXPORZIONISECONDI, uguale per tutti i
tipi di piatti.
La stazione dolce e caff`e ha invece una quantit`a illimitata di porzioni.

### 5.3 Processo operatore

All’inizio di ogni giornata lavorativa, l’operatore:

- compete con gli altri operatori per il posto presso la stazione a cui il responsabilemensa l’ha assegnato;
- se ne trova uno, comincia il proprio turno che terminer`a alla fine della giornata lavorativa;
- con un massimo di NOFPAUSE volte durante ciascuna giornata, l’operatore pu`o decidere (secondo un criterio
scelto dal programmatore) di interrompere il servizio per poi riprendere il proprio compito dopo la pausa.
Non tutti gli operatori possono andare in pausa contemporaneamente: non `e consentito agli operatori lasciare
  la stazione non presidiata (almeno un operatore deve sempre essere attivo). In caso un operatore decida di
  mettersi in pausa: - termina di servire il cliente che stava servendo; - lascia libera la stazione occupata; - aggiorna le statistiche.

Il processo operatore che al suo arrivo non trova una postazione libera resta in attesa che una postazione si liberi
(per una pausa di un altro operatore).

### 5.4 Processo operatorecassa

Il processo cassiere `e responsabile dei pagamenti: gli utenti che si presentano alla cassa indicano quali piatti hanno
scelto e se vogliono il caff`e, e il cassiere aggiorna il ricavo totale. Il costo di ciascun primo `e PRICEPRIMI, il costo
di ciascun secondo `e PRICESECONDI, e il prezzo del caff`e `e PRICECOFFEE.

### 5.5 Processo utente

I processi di tipo utente si recano presso la mensa. Pi`u in dettaglio, ogni giorno ogni processo utente:

- Stabilisce il menu che vuole acquistare (secondo un criterio stabilito dal progettista); ad esempio legge il
  file di configurazione menu.txt in cui sono elencate diverse alternative, e contiene almeno due primi e due
  secondi, oltre a dolce e caff`e. Il file di configurazione deve rispettare una sintassi definita dal progettista, e la
composizione del menu potr`a essere modificata in sede di esame;
- In base alla composizione del menu scelto –che deve includere almeno un primo, un secondo ed eventualmente
  il dolce-caff`e– si reca alle stazioni che forniscono i prodotti desiderati;
- Attende il proprio turno e l’erogazione del piatto presso ciascuna stazione di distribuzione;
- Al momento in cui l’utente sta per essere servito pu`o capitare che il piatto desiderato sia terminato: in questo
caso l’utente sceglier`a un altro piatto dello stesso tipo (primi o secondi). Se tutti i piatti di quel tipo sono
  terminati, manger`a solo secondo o solo primo. Se tutti i piatti di entrambi i tipi sono terminati, desiste ed
  esce;
- Dopo avere ottenuto i piatti scelti, passa in cassa e paga;
- Si siede in una stazionedirefezione dove consuma il pasto acquistato. All’interno della mensa esiste un
  numero di posti complessivo NOFTABLESEATS: per potersi sedere a un tavolo, l’utente deve verificare che ci
  sia almeno un posto libero; altrimenti attende che si liberi un posto.

Se al termine della giornata l’utente si trova ancora in coda, abbandona la mensa rinunciando al pasto. Il numero
di utenti che rinunciano `e uno dei parametri da monitorare.

### 5.6 Terminazione

La simulazione termina in una delle seguenti circorstanze:

timeout raggiungimento della durata impostata a SIMDURATION giorni;

overload numero di utenti in attesa al termine della giornata maggiore del valore OVERLOADTHRESHOLD.

Il gruppo di studenti deve produrre e consegnare, nell’archivio contenente il progetto, diverse configurazioni (file
configtimeout.conf e configoverload.conf) in grado di generare la terminazione nei casi sopra descritti.
Al termine della simulazione, l’output del programma deve riportare anche la causa di terminazione e le
statistiche finali.

## 6 Descrizione del progetto: versione “completa” (max 30)

In questa versione:

- Attraverso un nuovo eseguibile invocabile da linea di comando si crea un processo communicationdisorder
  che interrompe per un tempo STOPDURATION secondi il funzionamento dei pagamenti automatici, bloccando
  il funzionamento della stazionecassa;
- Attraverso un nuovo eseguibile invocabile da linea di comando, deve essere possibile aggiungere alla simulazione
  altri NNEWUSERS processi utente oltre a quelli inizialmente generati dal responsabilemensa;
- Tutte le statistiche prodotte devono anche essere salvate in un file testo di tipo csv, in modo da poter essere
  utilizzate per una analisi futura;
- Vengono distinti due tipi di utente: utentewticket and utentew/oticket. Le due figure sono simili, ma
  gli utenti con ticket hanno diritto a uno sconto, sono molto pi`u numerosi di quelli senza ticket (l’80% del
  totale degli utenti), ma fanno una fila per la lettura del ticket. Il progettista deve quindi individuare degli
  strumenti per descrivere questa ulteriore caratteristica e dar conto di questa variante (illustrando la soluzione
  proposta all’interno della relazione);

```
w_TICKET
```

```
w/o_TICKET
```

```
<latexit sha1_base64="q7VTnHSs0UkEIf96qFROkH4FLTM=">AAACCHicbVA9SwNBEN2LXzF+RS0tPAyCVbgTiZZBG8sI5gOScOxt5pIlu3fH7pwYj5Q2/hUbC0Vs/Ql2/hs3lxSa+GCGx3sz7M7zY8E1Os63lVtaXlldy68XNja3tneKu3sNHSWKQZ1FIlItn2oQPIQ6chTQihVQ6Qto+sOrid+8A6V5FN7iKIaupP2QB5xRNJJXPOwg3GOadcRUI30wOnS8WHHJx2OvWHLKTgZ7kbgzUiIz1LziV6cXsURCiExQrduuE2M3pQo5EzAudBINMWVD2oe2oSGVoLtpdsjYPjZKzw4iZSpEO1N/b6RUaj2SvpmUFAd63puI/3ntBIOLbsrDOEEI2fShIBE2RvYkFbvHFTAUI0MoU9z81WYDqihDk13BhODOn7xIGqdlt1Ku3JyVqpezOPLkgByRE+KSc1Il16RG6oSRR/JMXsmb9WS9WO/Wx3Q0Z8129skfWJ8/F8+bTw==</latexit>
stazioneprimi
<latexit sha1_base64="9o3B8pX4CwKyFJpucKRAQAhP2BA=">AAACCnicbVC7TsNAEDyHVwivACWNIUKiimyEAmUEDWWQyENKrOh82SSnnM/W3RoRrNQ0/AoNBQjR8gV0/A0XxwUkjLSr0cyu7nb8SHCNjvNt5ZaWV1bX8uuFjc2t7Z3i7l5Dh7FiUGehCFXLpxoEl1BHjgJakQIa+AKa/uhq6jfvQGkeylscR+AFdCB5nzOKRuoWDzsI95ikHTHRSB+MDp2uBhbKHp9MusWSU3ZS2IvEzUiJZKh1i1+dXsjiACQyQbVuu06EXkIVciZgUujEGiLKRnQAbUMlDUB7SXrKxD42Ss/uh8qURDtVf28kNNB6HPhmMqA41PPeVPzPa8fYv/ASLqMYQbLZQ/1Y2Bja01zsHlfAUIwNoUxx81ebDamiDE16BROCO3/yImmclt1KuXJzVqpeZnHkyQE5IifEJeekSq5JjdQJI4/kmbySN+vJerHerY/ZaM7KdvbJH1ifP6X1nCc=</latexit>
stazionesecondi
```

```
<latexit sha1_base64="gtyR/DlR8CPLR6LuDYwom33DOuM=">AAACCHicbVC7TgMxEPSFVwivACUFFhESVXSHUKCMoKEMEnlISXTyOXuJFd9D9h4inFLS8Cs0FCBEyyfQ8Tc4lxSQMJKt0czu2jteLIVG2/62ckvLK6tr+fXCxubW9k5xd6+ho0RxqPNIRqrlMQ1ShFBHgRJasQIWeBKa3vBq4jfvQGkRhbc4iqEbsH4ofMEZGsktHnYQ7jHNbsRUI3swOnRczrRm47FbLNllOwNdJM6MlMgMNbf41elFPAkgRC7NjLZjx9hNmULBJYwLnURDzPiQ9aFtaMgC0N00W2RMj43So36kzAmRZurvjpQFWo8Cz1QGDAd63puI/3ntBP2LbirCOEEI+fQhP5EUIzpJhfaEAo5yZAjjSpi/Uj5ginE02RVMCM78youkcVp2KuXKzVmpejmLI08OyBE5IQ45J1VyTWqkTjh5JM/klbxZT9aL9W59TEtz1qxnn/yB9fkD9e+bOQ==</latexit>
stazionecassa
```

```
<latexit sha1_base64="lKl4DQ19RhkZ/WY1u9Drc4nbdcE=">AAACCXicbVC7SgNBFJ31GeMramkzGASrsCsSLYM2lhHMA5KwzE7uJkNmH8zcFeOyrY2/YmOhiK1/YOffONmk0MQD93I4515m7vFiKTTa9re1tLyyurZe2Chubm3v7Jb29ps6ShSHBo9kpNoe0yBFCA0UKKEdK2CBJ6Hlja4mfusOlBZReIvjGHoBG4TCF5yhkdwS7SLcY5p3xFQjezA6dF0e+T5Alrmlsl2xc9BF4sxImcxQd0tf3X7EkwBC5JJp3XHsGHspUyi4hKzYTTTEjI/YADqGhiwA3UvzSzJ6bJQ+9SNlKkSaq783UhZoPQ48MxkwHOp5byL+53US9C96qQjjBCHk04f8RFKM6CQW2hcKOMqxIYwrYf5K+ZApxtGEVzQhOPMnL5LmacWpVqo3Z+Xa5SyOAjkkR+SEOOSc1Mg1qZMG4eSRPJNX8mY9WS/Wu/UxHV2yZjsH5A+szx+1OZug</latexit>
stazionecoffee
```

```
<latexit sha1_base64="KVpjBuiqJbH7MtRerW3j73bJaxg=">AAAB/3icbVDLSsNAFJ3UV62vqODGTbAIrkoiUl0W3bisYB/QhjKZTtqhk5kwc1MsMQt/xY0LRdz6G+78G6dpF9p64F4O59zL3DlBzJkG1/22Ciura+sbxc3S1vbO7p69f9DUMlGENojkUrUDrClngjaAAaftWFEcBZy2gtHN1G+NqdJMinuYxNSP8ECwkBEMRurZR12gD5DmHSAFPJacZVnPLrsVN4ezTLw5KaM56j37q9uXJImoAMKx1h3PjcFPsQJGOM1K3UTTGJMRHtCOoQJHVPtpfn/mnBql74RSmRLg5OrvjRRHWk+iwExGGIZ60ZuK/3mdBMIrP2UiToAKMnsoTLgD0pmG4fSZogT4xBBMFDO3OmSIFSZgIiuZELzFLy+T5nnFq1aqdxfl2vU8jiI6RifoDHnoEtXQLaqjBiLoET2jV/RmPVkv1rv1MRstWPOdQ/QH1ucPve6XOw==</latexit>
tavoli
```

```
1 2
```

```
3 4
```

```
5
```

```
Start
```

```
<latexit sha1_base64="VPq/T2hhXnC4kHXM9UM6rghBJIU=">AAACEHicbVA9SwNBEN3z2/gVtbQ5DKJVuBOJlqKNZQQTheQIe3uTuLgfx+6cGI78BBv/io2FIraWdv4bN+cVGn0ww+O9GXbnxangFoPg05uanpmdm19YrCwtr6yuVdc32lZnhkGLaaHNVUwtCK6ghRwFXKUGqIwFXMY3p2P/8haM5Vpd4DCFSNKB4n3OKDqpV93tItxhXnTEnGkpM1W63V7CrTYJmNGoV60F9aCA/5eEJamREs1e9aObaJZJUMgEtbYTBilGOTXImYBRpZtZSCm7oQPoOKqoBBvlxUEjf8cpid/XxpVCv1B/buRUWjuUsZuUFK/tpDcW//M6GfaPopyrNENQ7PuhfiZ81P44HT/hBhiKoSOUGe7+6rNraihDl2HFhRBOnvyXtPfrYaPeOD+oHZ+UcSyQLbJN9khIDskxOSNN0iKM3JNH8kxevAfvyXv13r5Hp7xyZ5P8gvf+BamdnuM=</latexit>
communicationdisorder
```

```
End
```

```
Figura 1: Schema del funzionamento della mensa.
```

- Gli utenti possono decidere di pranzare insieme: per mangiare insieme devono andare a pagare solo dopo
  essersi riforniti di tutti i piatti che desiderano acquistare. Nell’arco della simulazione ciascun gruppo `e costi-
tuito (attraverso l’estrazione di un numero casuale) da un numero compreso tra uno e MAXUSERSPERGROUP
utenti. L’associazione tra utenti e gruppi `e determinata al momento della creazione dei processi da parte del
  responsabilemensa;
- A differenza della versione minima, il tempo di refill dei primi e dei secondi `e AVGREFILLTIME, e dovr`a essere
  usato per generare un tempo casuale nell’intorno±20% del valore indicato.

```
Lo schema complessivo dell’interazione `e illustrato in Figura 1.
```

## 7 Configurazione

Tutti i parametri di configurazione sono letti a tempo di esecuzione, da file o da variabili di ambiente. Quindi,
un cambiamento dei parametri non deve determinare una nuova compilazione dei sorgenti (non `e consentito inserire
i parametri uno alla volta da terminale una volta avviata la simulazione). Il formato del file di configurazione `e a
scelta del progettista.

## 8 Requisiti implementativi

Il progetto (sia in versione “minima” che “completa”) deve

- evitare l’attesa attiva
- utilizzare almeno memoria condivisa, semafori e un meccanismo di comunicazione fra processi a scelta fra
  code di messaggi o pipe,
- essere realizzato sfruttando le tecniche di divisione in moduli del codice (per esempio, i vari processi devono
  essere lanciati da eseguibili diversi con execve(...)),
- essere compilato mediante l’utilizzo dell’utility make
- massimizzare il grado di concorrenza fra processi
- deallocare le risorse IPC che sono state allocate dai processi al termine della simulazione
- essere compilato con almeno le seguenti opzioni di compilazione:

```
gcc -Wvla -Wextra -Werror
```

- poter eseguire correttamente su una macchina (virtuale o fisica) che presenta parallelismo (due o pi`u proces-
  sori).

Per i motivi introdotti a lezione, ricordarsi di definire la macro GNUSOURCE o compilare il progetto con il flag
-DGNUSOURCE.
