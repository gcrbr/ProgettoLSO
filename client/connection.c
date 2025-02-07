#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "../common/protocol.h"
#endif

#ifndef MODELS_H
#define MODELS_H
#include "../common/models.h"
#endif

#ifndef GUI_H
#define GUI_H
#include "gui.h"
#endif

#ifndef CONNECTION_H
#define CONNECTION_H
#include "connection.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int player_id = -1;

struct Available_matches* availableMatches=NULL;

void clear_grid(char grid[3][3]) {
    memset(grid, 0, sizeof(grid[0][0]) * 9);
}

void clear_game_vars() {
    is_in_game_menu = 0;
    game_owned = 0;
    game_running = 0;
    curr_match_id = -1;
    other_player_play_again = -1;
    clear_grid(grid);
}

void handle_packet(int client, struct Packet *packet) {
    void *serialized = serialize_packet(packet);

    if(packet->id == SERVER_HANDSHAKE) {
        if(serialized != NULL) {
            struct Server_Handshake *_packet = (struct Server_Handshake *)serialized;
            player_id = _packet->player_id;
            if(DEBUG) {
                printf("%s Assegnato player_id=%d dal server\n", MSG_DEBUG, player_id);
            }
        }else {
            if(DEBUG) {
                printf("%s Ho ricevuto un pacchetto non serializzabile (id=%d)\n", MSG_DEBUG, packet->id);
            }
        }
    }

    if(packet->id == SERVER_MATCHREQUEST) {
        if(serialized != NULL ) {
            struct Server_MatchRequest *_packet = (struct Server_MatchRequest *)serialized;
            char risposta = 0;
            if(is_in_game_menu && !game_running) {
                printf("%s Il giocatore #%d vorrebbe unirsi alla tua partita #%d\n", MSG_INFO, _packet->other_player, _packet->match);
                do {
                    printf("Accetti? (S/N): ");
                    scanf(" %c", &risposta);
                } while(risposta != 'S' && risposta != 's' && risposta != 'N' && risposta != 'n');
                
                if(risposta == 'S' || risposta == 's') {
                    game_running = 1;
                    curr_match_id = _packet->match;
                }

                struct Client_ModifyRequest *modify = malloc(sizeof(struct Client_ModifyRequest));
                modify->accepted = (risposta == 'S' || risposta == 's');
                modify->match = _packet->match;
                struct Packet *new_packet = malloc(sizeof(struct Packet));
                new_packet->id = CLIENT_MODIFYREQUEST;
                new_packet->content = modify;
                send_packet(client, new_packet);
                free(new_packet);
            }else{
                struct Client_ModifyRequest *modify = malloc(sizeof(struct Client_ModifyRequest));
                modify->accepted = risposta;
                modify->match = _packet->match;
                struct Packet *new_packet = malloc(sizeof(struct Packet));
                new_packet->id = CLIENT_MODIFYREQUEST;
                new_packet->content = modify;
                send_packet(client, new_packet);
                free(new_packet);
            }
        }else{
            if(DEBUG) {
                printf("%s Ho ricevuto un pacchetto non serializzabile (id=%d)\n", MSG_DEBUG, packet->id);
            }
        }
    }

    if(packet->id == SERVER_UPDATEONREQUEST) {
        if(serialized != NULL) {
            struct Server_UpdateOnRequest *_packet = (struct Server_UpdateOnRequest *)serialized;
            accepted = _packet->accepted;
            if(_packet->accepted) {
                printf("%s L'altro giocatore ha accettato il tuo invito per la partita #%d\n", MSG_INFO, _packet->match);
                game_running = 1;
                curr_match_id = _packet->match;
            }else {
                printf("%s L'altro giocatore ha rifiutato il tuo invito per la partita #%d\n", MSG_INFO, _packet->match);
            }
        }else {
            if(DEBUG) {
                printf("%s Ho ricevuto un pacchetto non serializzabile (id=%d)\n", MSG_DEBUG, packet->id);
            }
        }
    }

    if(packet->id == SERVER_NOTICESTATE) {
        if(serialized != NULL) {
            struct Server_NoticeState *_packet = (struct Server_NoticeState *)serialized;
            game_state = _packet->state;
        }else {
            if(DEBUG) {
                printf("%s Ho ricevuto un pacchetto non serializzabile (id=%d)\n", MSG_DEBUG, packet->id);
            }
        }
    }

    if(packet->id == SERVER_NOTICEMOVE) {
        if(serialized != NULL) {
            struct Server_NoticeMove *_packet = (struct Server_NoticeMove *)serialized;
            grid[_packet->moveX][_packet->moveY] = game_owned ? 2 : 1;
        }else {
            if(DEBUG) {
                printf("%s Ho ricevuto un pacchetto non serializzabile (id=%d)\n", MSG_DEBUG, packet->id);
            }
        }
    }

    if(packet->id == SERVER_NOTICEPLAYAGAIN) {
        if(serialized != NULL) {
            struct Server_NoticePlayAgain *_packet = (struct Server_NoticePlayAgain *)serialized;
            if(_packet->choice) {
                printf("\nIl giocatore ha accettato la rivincita\n");
            }
            other_player_play_again = _packet->choice;
        }else {
            if(DEBUG) {
                printf("%s Ho ricevuto un pacchetto non serializzabile (id=%d)\n", MSG_DEBUG, packet->id);
            }
        }
    }

    if(packet->id == SERVER_BROADCASTMATCH){
        if(serialized != NULL) {
            struct Server_BroadcastMatch *_packet=(struct Server_BroadcastMatch *)serialized;
            add_node((struct generic_node **)&availableMatches, (void *)_packet);
        }else {
            if(DEBUG) {
                printf("%s Ho ricevuto un pacchetto non serializzabile (id=%d)\n", MSG_DEBUG, packet->id);
            }
        }
    }

    if(packet->id == SERVER_BROADCASTREMOVEMATCH){
        if(serialized != NULL) {
            struct Server_BroadcastRemoveMatch *_packet=(struct Server_BroadcastRemoveMatch *)serialized;
            struct Server_BroadcastMatch* node_to_del = find_node(availableMatches, _packet->match);
            if(node_to_del!=NULL){
                remove_node((struct generic_node**)&availableMatches, (void *)node_to_del);
            }
        }else {
            if(DEBUG) {
                printf("%s Ho ricevuto un pacchetto non serializzabile (id=%d)\n", MSG_DEBUG, packet->id);
            }
        } 
    }

    if(player_id == -1) { // Non si procede se non mi è stato assegnato un player id
        return;
    }
}

struct Server_BroadcastMatch* find_node(struct Available_matches* head, int match){
    if(head==NULL){
        return NULL;
    }
    struct Available_matches * curr = head;
    while (curr!=NULL){
        if(curr->broad->match==match){
                return curr->broad;
        }
        curr=curr->next;
    }
    return NULL;
}

void print_available_matches(){
    struct Available_matches* curr=availableMatches;
    while(curr!=NULL){
        struct Server_BroadcastMatch* broadc=curr->broad;
        printf("id match: %d , id player: %d\n", broadc->match, broadc->player_id);
        curr=curr->next;
    }
}

void create_match(int sockfd) {
    struct Packet *packet = malloc(sizeof(struct Packet));
    packet->id = CLIENT_CREATEMATCH;
    send_packet(sockfd, packet);
    free(packet);
}

void join_match(int sockfd, int match) {
    struct Client_JoinMatch *join = malloc(sizeof(struct Client_JoinMatch));
    join->match = match;
    struct Packet *packet = malloc(sizeof(struct Packet));
    packet->id = CLIENT_JOINMATCH;
    packet->content = join;
    send_packet(sockfd, packet);
    free(packet);
}

void make_move(int sockfd, int moveX, int moveY, int match) {
    struct Client_MakeMove *move = malloc(sizeof(struct Client_MakeMove));
    move->moveX = moveX;
    move->moveY = moveY;
    move->match = match;
    struct Packet *packet = malloc(sizeof(struct Packet));
    packet->id = CLIENT_MAKEMOVE;
    packet->content = move;
    send_packet(sockfd, packet);
    free(packet);
}

void send_play_again(int sockfd, int choice, int match) {
    struct Client_PlayAgain *play = malloc(sizeof(struct Client_PlayAgain));
    play->choice = choice;
    play->match = match;
    struct Packet *packet = malloc(sizeof(struct Packet));
    packet->id = CLIENT_PLAYAGAIN;
    packet->content = play;
    send_packet(sockfd, packet);
    free(packet);
}