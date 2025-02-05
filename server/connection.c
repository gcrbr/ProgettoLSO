#ifndef CONNECTION_H
#define CONNECTION_H
#include "connection.h"
#endif

#ifndef MODELS_H
#define MODELS_H
#include "../common/models.h"
#endif

#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "../common/protocol.h"
#endif

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

void *joiner_thread(void *args) {
    struct joiner_thread_args *thread_args = (struct joiner_thread_args*)args;
    pthread_join((pthread_t)(thread_args->thread), NULL);
    printf("%s Connessione chiusa (fd=%d)\n", MSG_INFO, thread_args->client->conn);
    remove_node((struct generic_node **)&clients, (void*)(thread_args->client)); // Rimuovi il client dalla lista
    remove_matches_by_player(&matches, thread_args->client->conn); // Rimuovi tutte le partite create dal giocatore
    free(thread_args->client);
    return NULL;
}

// Lo usiamo per mandare i pacchetti che hanno size=0 e content=NULL, inutile ripetere il codice dell'allocazione nel codice
void send_empty_packet(struct client *client, int packet_id){
    struct Packet *new_packet = malloc(sizeof(struct Packet));
    new_packet->id = packet_id;
    send_packet(client->conn, new_packet);
    free(new_packet);
}

void handle_packet(struct client *client, struct Packet *packet) {
    // Usiamo il file descriptor come player id
    int player_id = (client->conn) % 255;
    struct Player *player = malloc(sizeof(struct Player)); // Rivedere, viene allocato un nuovo player ad ogni pacchetto...
    player->id = player_id;

    // Serializziamo i pacchetti in ingresso per trasformarli in struct
    void *serialized = serialize_packet(packet);

    if(DEBUG) {
        printf("%s Packet(fd=%d, id=%d, size=%d, content=%s)\n", MSG_DEBUG, player_id, packet->id, packet->size, (char*)packet->content);
    }

    if(packet->id == CLIENT_HANDSHAKE) {
        struct Server_Handshake *handshake = malloc(sizeof(struct Server_Handshake));
        handshake->player_id = player_id;
        struct Packet *new_packet = malloc(sizeof(struct Packet));
        new_packet->id = SERVER_HANDSHAKE;
        new_packet->content = handshake;
        send_packet(client->conn, new_packet);
        free(new_packet);
    }

    if(packet->id == CLIENT_CREATEMATCH) {
        if(currMatchSize < MAX_MATCHES) {
            struct Match *new_match = malloc(sizeof(struct Match));
            new_match->participants[0] = player;
            new_match->state = STATE_CREATED;
            new_match->freeSlots = 9;

            add_node((struct generic_node **)&matches, (void *)new_match);
            ++currMatchSize;
            
            send_empty_packet(client, SERVER_SUCCESS);

            if(DEBUG) {
                printf("%s Il Player id=%d ha creato una nuova partita\n", MSG_DEBUG, player_id);
            }
        }else{
            printf("%s Raggiunto limite di Match possibili (%d)\n", MSG_WARNING, MAX_MATCHES);
            send_empty_packet(client, SERVER_ERROR);
        }
    }

    if(packet->id == CLIENT_JOINMATCH) {
        if(serialized != NULL) {
            struct Client_JoinMatch *_packet = (struct Client_JoinMatch *)serialized;
            struct Match *found_match;
            if((found_match = get_match_by_id(matches, _packet->match)) != NULL) {
                if(found_match->participants[0]->id != player_id) {
                    found_match->requester = player;
                    if(DEBUG) {
                        printf("%s Il Player id=%d ha inviato una richiesta al Match id=%d (creato da Player id=%d)\n", MSG_DEBUG, player_id, _packet->match, found_match->participants[0]->id);
                    }
                    send_empty_packet(client, SERVER_SUCCESS);
                    
                    struct Server_MatchRequest *request_packet = malloc(sizeof(struct Server_MatchRequest));
                    request_packet->other_player = player_id;
                    request_packet->match = _packet->match;
                    struct Packet *new_packet = malloc(sizeof(struct Packet));
                    new_packet->id = SERVER_MATCHREQUEST;
                    new_packet->content = request_packet;
                    send_packet(found_match->participants[0]->id, new_packet); // Mando la richiesta al client dell'altro giocatore
                    free(new_packet);
                }else {
                    if(DEBUG) {
                        printf("%s Il Player id=%d ha provato ad entrare nel suo stesso Match id=%d\n", MSG_DEBUG, player_id, _packet->match);
                    }
                    send_empty_packet(client, SERVER_ERROR);
                }
            }else {
                if(DEBUG) {
                    printf("%s Player id=%d ha provato ad entrare in un Match id=%d non valido\n", MSG_DEBUG, player_id, _packet->match);
                }
                send_empty_packet(client, SERVER_ERROR);
            }
        }else {
            if(DEBUG) {
                printf("%s Player id=%d ha inviato un Packet id=%d non serializzabile\n", MSG_DEBUG, player_id, packet->id);
            }
        }
    }

    if(packet->id == CLIENT_MODIFYREQUEST) {
        if(serialized != NULL) {
            struct Client_ModifyRequest *_packet = (struct Client_ModifyRequest *)serialized;
            struct Match *found_match;

            if((found_match = get_match_by_id(matches, _packet->match)) != NULL) {
                if(found_match->participants[0]->id == player_id && found_match->requester != NULL) {
                    if(_packet->accepted) {
                        found_match->participants[1] = found_match->requester;
                        found_match->requester = NULL;
                        found_match->state = STATE_TURN_PLAYER1; // Inizia il creatore della partita

                        struct Server_NoticeState *turn_packet = malloc(sizeof(struct Server_NoticeState));
                        turn_packet->match = _packet->match;
                        turn_packet->state = STATE_TURN_PLAYER1;
                        struct Packet *new_packet = malloc(sizeof(struct Packet));
                        new_packet->id = SERVER_NOTICESTATE;
                        new_packet->content = turn_packet;
                        send_packet(client->conn, new_packet); // Avviso il creatore della partita che è il suo turno

                        if(DEBUG) {
                            printf("%s Player id=%d ha accettato la richiesta del Player id=%d per il Match id=%d\n", MSG_DEBUG, player_id, found_match->participants[1]->id, _packet->match);
                        }
                    }else {
                        if(DEBUG) {
                            printf("%s Player id=%d ha rifiutato la richiesta del Player id=%d per il Match id=%d\n", MSG_DEBUG, player_id, found_match->requester->id, _packet->match);
                        }
                        found_match->requester = NULL;
                        send_empty_packet(client, SERVER_ERROR);
                    }
                }else {
                    if(DEBUG) {
                        printf("%s Player id=%d ha provato a modificare il Match id=%d in un modo non valido\n", MSG_DEBUG, player_id, _packet->match);
                    }
                    send_empty_packet(client, SERVER_ERROR);
                }
            }else {
                if(DEBUG) {
                    printf("%s Player id=%d ha provato a modificare un Match id=%d non valido\n", MSG_DEBUG, player_id, _packet->match);
                }
                send_empty_packet(client, SERVER_ERROR);
            }
        }else {
            if(DEBUG) {
                printf("%s Player id=%d ha inviato un Packet id=%d non serializzabile\n", MSG_DEBUG, player_id, packet->id);
            }
        }
    }

    if(packet->id == CLIENT_MAKEMOVE) {
        if(serialized != NULL) {
            struct Client_MakeMove *_packet = (struct Client_MakeMove *)serialized;
            struct Match *found_match;

            if(_packet->moveX >= 0 && _packet->moveX <= 2 && _packet->moveY >= 0 && _packet->moveY <= 2) {
                if((found_match = get_match_by_id(matches, _packet->match)) != NULL) {
                    // Controllo se il giocatore è uno dei due della partita
                    if(found_match->participants[0]->id == player_id || found_match->participants[1]->id == player_id) {
                        // Controllo se è il turno di uno o dell'altro
                        if(
                            found_match->state == STATE_TURN_PLAYER1 && found_match->participants[0]->id == player_id ||
                            found_match->state == STATE_TURN_PLAYER2 && found_match->participants[1]->id == player_id
                        ) {
                            int moving_player_id = found_match->state == STATE_TURN_PLAYER1 ? found_match->participants[0]->id : found_match->participants[1]->id;
                            //printf("state=%d,expected_player=%d,actual_player=%d\n", found_match->state, moving_player_id, player_id);
                            // Aggiorno la griglia, controllando prima se la casella è libera
                            if(found_match->grid[_packet->moveX][_packet->moveY] == 0) {
                                int other_player_id = player_id == found_match->participants[0]->id ? found_match->participants[1]->id : found_match->participants[0]->id;

                                found_match->grid[_packet->moveX][_packet->moveY] = found_match->state == STATE_TURN_PLAYER1 ? 1 : 2;
                                found_match->freeSlots--; // Tolgo una casella

                                struct Packet *notice_move = malloc(sizeof(struct Packet));
                                notice_move->id = SERVER_NOTICEMOVE;
                                // Siccome il pacchetto è praticamente identico a quello che manda il client posso serializzaro a quello server
                                notice_move->content = (struct Server_NoticeMove *)_packet;
                                // Avviso l'altro giocatore della mossa, non il corrente - al corrente mando success
                                send_packet(other_player_id, notice_move);
                                free(notice_move);

                                if(DEBUG) {
                                    printf("%s Il Player id=%d ha fatto una Mossa(%d, %d) nel Match id=%d contro il Player id=%d\n", MSG_DEBUG, moving_player_id, _packet->moveX, _packet->moveY, _packet->match, found_match->participants[1]->id);
                                    char symbols[3] = {'-', 'X', 'O'};
                                    printf("%s Griglia attuale Match id=%d\n%c %c %c\n%c %c %c\n%c %c %c\n", MSG_DEBUG, _packet->match,
                                        symbols[found_match->grid[0][0]], symbols[found_match->grid[1][0]], symbols[found_match->grid[2][0]],
                                        symbols[found_match->grid[0][1]], symbols[found_match->grid[1][1]], symbols[found_match->grid[2][1]],
                                        symbols[found_match->grid[0][2]], symbols[found_match->grid[1][2]], symbols[found_match->grid[2][2]]
                                    );
                                }

                                int new_state = found_match->state == STATE_TURN_PLAYER1 ? STATE_TURN_PLAYER2 : STATE_TURN_PLAYER1;
                                found_match->state = new_state;

                                struct Server_NoticeState *notice_state = malloc(sizeof(struct Server_NoticeState));
                                notice_state->state = new_state;
                                struct Packet *notice_turn = malloc(sizeof(struct Packet));
                                notice_turn->id = SERVER_NOTICESTATE;
                                notice_turn->content = notice_state;
                                send_packet(other_player_id, notice_turn);
                                free(notice_turn);

                                send_empty_packet(client, SERVER_SUCCESS);

                                // Controllo se qualcuno ha vinto
                                /*
                                    Vincenti
                                    (0, 0), (1, 0), (2, 0)
                                    (0, 1), (1, 1), (2, 1)	Orizzontali
                                    (0, 2), (1, 2), (2, 2)

                                    (0, 0), (0, 1), (0, 2)
                                    (1, 0), (1, 1), (1, 2)	Verticali
                                    (2, 0), (2, 1), (2, 2)

                                    (0, 0), (1, 1), (2, 2)	Diagonali
                                    (2, 0), (1, 1), (0, 2)
                                */
                                for(int i = 1; i < 3; ++i) { // Lo faccio per i valori 1, 2 cioè X e Cerchio
                                    if(
                                        (found_match->grid[0][0] == i && found_match->grid[1][0] == i && found_match->grid[2][0] == i) ||
                                        (found_match->grid[0][1] == i && found_match->grid[1][1] == i && found_match->grid[2][1] == i) ||
                                        (found_match->grid[0][2] == i && found_match->grid[1][2] == i && found_match->grid[2][2] == i) ||

                                        (found_match->grid[0][0] == i && found_match->grid[0][1] == i && found_match->grid[0][2] == i) ||
                                        (found_match->grid[1][0] == i && found_match->grid[1][1] == i && found_match->grid[1][2] == i) ||
                                        (found_match->grid[2][0] == i && found_match->grid[2][1] == i && found_match->grid[2][2] == i) ||

                                        (found_match->grid[0][0] == i && found_match->grid[1][1] == i && found_match->grid[2][2] == i) ||
                                        (found_match->grid[2][0] == i && found_match->grid[1][1] == i && found_match->grid[2][0] == i)
                                    ) {
                                        found_match->state = STATE_TERMINATED; // Potrei evitarlo se alla fine la elimino...

                                        struct Server_NoticeState *notice_packet = malloc(sizeof(struct Server_NoticeState));
                                        notice_packet->match = _packet->match;

                                        struct Packet *new_packet = malloc(sizeof(struct Packet));
                                        new_packet->id = SERVER_NOTICESTATE;
                                        new_packet->content = notice_packet;

                                        notice_packet->state = STATE_WIN;
                                        send_packet(found_match->participants[i - 1]->id, new_packet); // Giocatore che ha vinto
                                        notice_packet->state = STATE_LOSE;
                                        send_packet(found_match->participants[i % 2]->id, new_packet); // Giocatore che ha perso

                                        if(DEBUG) {
                                            printf("%s Il Player id=%d ha vinto il Match id=%d contro il Player id=%d\n", MSG_DEBUG, found_match->participants[i - 1]->id, _packet->match, found_match->participants[i % 2]->id);
                                        }

                                        free(new_packet);
                                        remove_node((struct generic_node **)&matches, found_match);
                                    }
                                }
                            
                                // Controllo se siamo in pareggio; cioè se nessuno ha vinto ma sono finite le caselle
                                if(found_match->freeSlots == 0) {
                                    found_match->state = STATE_TERMINATED;

                                    struct Server_NoticeState *notice_packet = malloc(sizeof(struct Server_NoticeState));
                                    notice_packet->state = STATE_DRAW;
                                    notice_packet->match = _packet->match;

                                    struct Packet *new_packet = malloc(sizeof(struct Packet));
                                    new_packet->id = SERVER_NOTICESTATE;
                                    new_packet->content = notice_packet;

                                    // Avviso entrambi che è un pareggio ed elimino la partita
                                    send_packet(found_match->participants[0]->id, new_packet);
                                    send_packet(found_match->participants[1]->id, new_packet);

                                    if(DEBUG) {
                                        printf("%s Il Match id=%d tra i Player id=%d,%d è terminata in pareggio\n", MSG_DEBUG, _packet->match, found_match->participants[0]->id, found_match->participants[1]->id);
                                    }

                                    free(new_packet);
                                    remove_node((struct generic_node **)&matches, found_match);
                                }
                            }else {
                                if(DEBUG) {
                                    printf("%s Il Player id=%d ha provato a fare una Mossa(%d,%d) in una casella occupata nel Match id=%d\n", MSG_DEBUG, moving_player_id, _packet->moveX, _packet->moveY, _packet->match);
                                }
                                send_empty_packet(client, SERVER_ERROR);
                            }
                        }else {
                            if(DEBUG) {
                                printf("%s Il Player id=%d ha provato a fare una Mossa(%d,%d) nel Match id=%d al di fuori del suo turno\n", MSG_DEBUG, player_id, _packet->moveX, _packet->moveY, _packet->match);
                            }
                            send_empty_packet(client, SERVER_ERROR);
                        }
                    }else {
                        if(DEBUG) {
                            printf("%s Il Player id=%d ha provato a fare una mossa in un Match id=%d in cui non sta giocando\n", MSG_DEBUG, player_id, _packet->match);
                        }
                        send_empty_packet(client, SERVER_ERROR);
                    }
                }else {
                    if(DEBUG) {
                        printf("%s Il Player id=%d ha provato a fare una mossa in un Match id=%d non valido\n", MSG_DEBUG, player_id, _packet->match);
                    }
                    send_empty_packet(client, SERVER_ERROR);
                }
            }else {
                if(DEBUG) {
                    printf("%s Il Player id=%d ha provato a fare una Mossa(%d,%d) non valida\n", MSG_DEBUG, player_id, _packet->moveX, _packet->moveY);
                }
                send_empty_packet(client, SERVER_ERROR);
            }
        }else {
            if(DEBUG) {
                printf("%s Player id=%d ha inviato un Packet id=%d non serializzabile\n", MSG_DEBUG, player_id, packet->id);
            }
        }
    }
}

void read_data(struct client *client, char buf[BUFFER_SIZE]) {
    struct Packet *packet = malloc(sizeof(struct Packet));
    packet->id = buf[0];
    packet->size = 0;
    packet->size = buf[1] + (buf[2] << 8); // Little endian Int -> C Int
    packet->content = malloc(sizeof(char) * packet->size);
    memcpy(packet->content, buf + 3, packet->size);
    handle_packet(client, packet);
    free(packet->content);
    free(packet);
}

void *server_thread(void *args) {
    struct client *client = (struct client *)args;
    char buf[BUFFER_SIZE];
    while(1) {
        if(recv(client->conn, buf, BUFFER_SIZE, 0) == 0) {
            break;
        }else {
            read_data(client, buf);
        }
    }
    return NULL;
}