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

void send_empty_packet(struct client *client, int packet_id){
    struct Packet *new_packet = malloc(sizeof(struct Packet));
    new_packet->id = packet_id;
    send_packet(client->conn, new_packet);
    free(new_packet);
}

void handle_packet(struct client *client, struct Packet *packet) {
    // Usiamo il file descriptor come player id
    int player_id = (client->conn) % 255;
    struct Player *player = malloc(sizeof(struct Player));
    player->id = player_id;

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
        struct Match *new_match = malloc(sizeof(struct Match));
        new_match->participants[0] = player;
        new_match->state = STATE_CREATED;

        add_node((struct generic_node **)&matches, (void *)new_match);
        
        send_empty_packet(client, SERVER_SUCCESS);

        if(DEBUG) {
            printf("%s Created new match by Player id=%d\n", MSG_DEBUG, player_id);
        }
    }

    if(packet->id == CLIENT_JOINMATCH) {
        int requested_match_id = *((int *)(packet->content));
        struct Match *found_match;
        if((found_match = get_match_by_id(matches, requested_match_id)) != NULL) {
            if(found_match->participants[0]->id != player_id) {
                found_match->requester = player;
                if(DEBUG) {
                    printf("%s Player id=%d sent a request for Match id=%d (owned by Player id=%d)\n", MSG_DEBUG, player_id, requested_match_id, found_match->participants[0]->id);
                }
                send_empty_packet(client, SERVER_SUCCESS);
                
                struct Server_MatchRequest *request_packet = malloc(sizeof(struct Server_MatchRequest));
                request_packet->other_player = player_id;
                request_packet->match = requested_match_id;
                struct Packet *new_packet = malloc(sizeof(struct Packet));
                new_packet->id = SERVER_MATCHREQUEST;
                new_packet->content = request_packet;
                send_packet(found_match->participants[0]->id, new_packet); // Mando la richiesta al client dell'altro giocatore
                free(new_packet);
            }else {
                if(DEBUG) {
                    printf("%s Player id=%d tried to join its own Match id=%d\n", MSG_DEBUG, player_id, requested_match_id);
                }
                send_empty_packet(client, SERVER_ERROR);
            }
        }else {
            if(DEBUG) {
                printf("%s Player id=%d tried to join invalid Match id=%d\n", MSG_DEBUG, player_id, requested_match_id);
            }
            send_empty_packet(client, SERVER_ERROR);
        }
    }

    if(packet->id == CLIENT_ACCEPTREQUEST || packet->id == CLIENT_REJECTREQUEST) {
        int match_id = *((int *)(packet->content));
        struct Match *found_match;
        if((found_match = get_match_by_id(matches, match_id)) != NULL) {
            if(found_match->participants[0]->id == player_id && found_match->requester != NULL) {
                if(packet->id == CLIENT_ACCEPTREQUEST) {
                    found_match->participants[1] = found_match->requester;
                    found_match->requester = NULL;
                    found_match->state = STATE_TURN_PLAYER1; // Inizia il creatore della partita

                    send_empty_packet(client, SERVER_SUCCESS);

                    if(DEBUG) {
                        printf("%s Player id=%d accepted the request made by id=%d for Match id=%d\n", MSG_DEBUG, player_id, found_match->participants[1]->id, match_id);
                    }
                }

                if(packet->id == CLIENT_REJECTREQUEST) {
                    found_match->requester = NULL;

                    send_empty_packet(client, SERVER_ERROR);

                    if(DEBUG) {
                        printf("%s Player id=%d reject the request made by id=%d for Match id=%d\n", MSG_DEBUG, player_id, found_match->participants[1]->id, match_id);
                    }
                }
            }else {
                if(DEBUG) {
                    printf("%s Player id=%d tried to modify a request for a Match in an invalid way id=%d\n", MSG_DEBUG, player_id, match_id);
                }
                send_empty_packet(client, SERVER_ERROR);
            }
        }else {
            if(DEBUG) {
                printf("%s Player id=%d tried to modify a request for an invalid Match id=%d\n", MSG_DEBUG, player_id, match_id);
            }
            send_empty_packet(client, SERVER_ERROR);
        }
    }
}

void read_data(struct client *client, char buf[BUFFER_SIZE]) {
    struct Packet *packet = malloc(sizeof(struct Packet));
    packet->id = buf[0];
    packet->size = 0;
    memcpy(&(packet->size), buf + 1, 2);
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