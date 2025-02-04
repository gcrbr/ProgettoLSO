struct Packet {
    int id;
    int size;
    void *content;
};

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
#define CLIENT_ACCEPTREQUEST     4
#define CLIENT_REJECTREQUEST     5

struct Client_JoinMatch {
    int match;
};

struct Client_AcceptRequest {
    int match;
};

/* 
    Server packets
    (Server->Client)
*/

#define SERVER_HANDSHAKE         20
#define SERVER_SUCCESS           21
#define SERVER_ERROR             22
#define SERVER_MATCHREQUEST      23
#define SERVER_NOTICETURN        24 // Avvisa che è il turno del giocatore che lo riceve

struct Server_Handshake {
    int player_id;
};

struct Server_MatchRequest {
    int other_player;   // Giocatore che ha fatto la richiesta
    int match;          // Partita
};
