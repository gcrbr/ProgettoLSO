# Progetto LSO
Lo studente dovrà realizzare un server multi-client per giocare a Tris (con due giocatori per ogni
partita). Un giocatore può creare una o più partite, ma può giocare solo ad una partita alla volta. Il
creatore di una partita può accettare o rifiutare la richiesta di partecipazione alla partita da un nuovo
giocatore. Gli stati di gioco possono essere terminata, in corso, in attesa, nuova creazione. Gli stati
di terminazione di partita possono essere vittoria, sconfitta, pareggio rispetto al giocatore. In base
allo stato di gioco di ogni singola partita, tutti i giocatori collegati al server dovranno ricevere un
messaggio diverso. Per esempio, "in attesa" tutti i giocatori vengono invitati a partecipare alla
partita. Le partite devono essere identificate in maniera univoca. A fine partita (terminata), i
giocatori di ogni partita possono scegliere se iniziare o meno un'altra partita.

- Opzionale: Il vincitore di una partita può decidere se fare un'altra partita - in questo caso, se
non era il proprietario, diventa il proprietario della partita e attende un nuovo giocatore. Il
perdente deve lasciare la partita. Se c'è pareggio, entrambi i giocatori possono decidere se
farne un'altra.