#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "protocol.h"
#endif

#ifndef MODELS_H
#define MODELS_H
#include "models.h"
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>

// Utilizzato per i pacchetti in ingresso da entrambi i lati
// Uso void* perché tipo generico, non so a priori quale pacchetto dovrò ritornare. Dove chiamo faccio poi il casting
// (SERIALIZZAZIONE IN ENTRATA, byte array->struct)
void *serialize_packet(struct Packet *packet) {
    if(packet->id == CLIENT_JOINMATCH) {
        if(packet->size < 1)    return NULL;
        struct Client_JoinMatch *new = malloc(sizeof(struct Client_JoinMatch));
        new->match = *((int *)packet->content);
        return new;
    }

    if(packet->id == CLIENT_MODIFYREQUEST) {
        if(packet->size < 2)    return NULL;
        struct Client_ModifyRequest *new = malloc(sizeof(struct Client_ModifyRequest));
        new->accepted = ((int *)packet->content)[0];
        new->match = ((int *)packet->content)[1];
        return new;
    }

    if(packet->id == CLIENT_MAKEMOVE) {
        if(packet->size < 3)    return NULL;
        struct Client_MakeMove *new = malloc(sizeof(struct Client_MakeMove));
        // Uso char perché è lungo 1 byte a differenza di int (4), altrimenti prende anche i campi che non sono suoi
        new->moveX = ((char *)packet->content)[0];
        new->moveY = ((char *)packet->content)[1];
        new->match = ((char *)packet->content)[2];
        return new;
    }

    return NULL;
}

// Utilizzato per i pacchetti in uscita da entrambi i lati 
// (SERIALIZZAZIONE IN USCITA, struct->byte array)
void send_packet(int sockfd, struct Packet *packet) {
    /*
        Un pacchetto può essere al massimo lungo 65536 + 3 perché:
        - Il numero di byte che compongono l'id del pacchetto è di 1
        - Il numero di byte che compongono l'intero che definisce la lunghezza del pacchetto è di 2
        - La lunghezza massima ottenibile da un intero a 2 byte è 2^(8*2)=2^16=65536
    */
    char serialized[65536 + 3];
    serialized[0] = packet->id;
    
    /*
        Serializzo il contenuto in base al singolo pacchetto
        Non mando lo struct come byte array perché le differenze tra CPU di client e server
        potrebbero creare problemi.
    */
    if(packet->id == SERVER_HANDSHAKE) {
        packet->size = 1;
        serialized[3] = ((struct Server_Handshake *)packet->content)->player_id;
    }
    
    if(packet->id == SERVER_SUCCESS || packet->id == SERVER_ERROR) {
        packet->size = 0;
    }

    if(packet->id == SERVER_MATCHREQUEST) {
        packet->size = 2;
        serialized[3] = ((struct Server_MatchRequest *)packet->content)->other_player;
        serialized[4] = ((struct Server_MatchRequest *)packet->content)->match; // Se l'id del match supera 255 esplode
    }

    if(packet->id == SERVER_NOTICESTATE) {
        packet->size = 2;
        serialized[3] = ((struct Server_NoticeState *)packet->content)->state;
        serialized[4] = ((struct Server_NoticeState *)packet->content)->match;
    }

    if(packet->id == SERVER_NOTICEMOVE) {
        packet->size = 3;
        serialized[3] = ((struct Server_NoticeMove *)packet->content)->moveX;
        serialized[4] = ((struct Server_NoticeMove *)packet->content)->moveY;
        serialized[5] = ((struct Server_NoticeMove *)packet->content)->match;
    }

    if(packet->id == SERVER_BROADCASTMATCH) {
        packet->size = 2;
        serialized[3] = ((struct Server_BroadcastMatch *)packet->content)->player_id;
        serialized[4] = ((struct Server_BroadcastMatch *)packet->content)->match;
    }

    /*
        Conversione da int a array di byte (little endian), usato per la lunghezza del pacchetto
        Es: 25  -> '\x19\x00'
            300 -> '\x2c\x01'
        Vedi: https://stackoverflow.com/questions/21396032/2-byte-array-in-little-endian-to-int-without-java-nio
    */
    serialized[1] = packet->size & 0xFF;
    serialized[2] = (packet->size >> 8) & 0xFF;

    /*
        Invio il pacchetto. 
        Come lunghezza considero packet->size + 3 perché è la vera lunghezza rispetto alla massima
    */
    if(send(sockfd, serialized, packet->size + 3, 0) < 1) {
        fprintf(stderr, "%s Impossibile inviare pacchetto (fd=%d, id=%d, size=%d): %s\n", 
        MSG_ERROR, sockfd, packet->id, packet->size, strerror(errno));
    }
}