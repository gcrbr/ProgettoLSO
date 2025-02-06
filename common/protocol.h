struct Packet {
    int id;
    int size;
    void *content;
};

void *serialize_packet(struct Packet *packet);
void send_packet(int sockfd, struct Packet *packet);

// Nota: Per i pacchetti che non hanno campi (quindi size=0) non ho creato le struct perché sarebbero vuote

/*
    Client packets
    (Client->Server)
*/

#define CLIENT_HANDSHAKE         0
#define CLIENT_CREATEMATCH       1
#define CLIENT_GETFREEMATCHES    2
#define CLIENT_JOINMATCH         3
#define CLIENT_MODIFYREQUEST     4
#define CLIENT_MAKEMOVE          5
#define CLIENT_PLAYAGAIN         6

struct Client_JoinMatch {
    int match;
};

struct Client_ModifyRequest { // Server per accettare o rifiutare, era inutile fare due pacchetti separati
    int accepted;             // 0 = No, 1 = Sì
    int match;
};

struct Client_MakeMove {      // Pacchetto per fare una mossa in una partita
    int moveX;
    int moveY;
    int match;
};

struct Client_PlayAgain {
    int choice;         // 0 = No, 1 = Sì (se vuole rigiocare)
    int match;          // Id della partita
};

/* 
    Server packets
    (Server->Client)
*/

#define SERVER_HANDSHAKE         20
#define SERVER_SUCCESS           21
#define SERVER_ERROR             22
#define SERVER_MATCHREQUEST      23
#define SERVER_UPDATEONREQUEST   30
#define SERVER_NOTICESTATE       24 // Avvisa i client degli stati della partita, tra cui anche i turni
#define SERVER_NOTICEMOVE        25 // Avvisa l'altro giocatore della mossa fatta dall'altro
#define SERVER_BROADCASTMATCH    26 // Avvisa a tutti i client connessi della nuova partita creata
#define SERVER_NOTICEPLAYAGAIN   27 // Avvisa i due client della nuova partita creata (se hanno detto entrambi sì)

struct Server_Handshake {
    int player_id;
};

struct Server_MatchRequest {
    int other_player;   // Giocatore che ha fatto la richiesta
    int match;          // Partita
};

struct Server_UpdateOnRequest {
    int accepted;
    int match;
};

struct Server_NoticeState {
    int state;
    int match;
};

struct Server_NoticeMove {
    int moveX;
    int moveY;
    int match;
};

struct Server_BroadcastMatch {
    int player_id;
    int match;
};

struct Server_NoticePlayAgain { // Il corrispondente che avvisa l'altro client se è stato accettato o meno
    int choice;
    int match;
};