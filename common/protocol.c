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