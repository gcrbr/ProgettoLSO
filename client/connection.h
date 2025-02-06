#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "../common/protocol.h"
#endif

extern int player_id;

void handle_packet(int client, struct Packet *packet);
void create_match(int sockfd);
void join_match(int sockfd, int match);
void make_move(int sockfd, int moveX, int moveY, int match);
void send_play_again(int sockfd, int choice, int match);
void clear_grid(char grid[3][3]);
void clear_game_vars();