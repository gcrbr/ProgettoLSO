#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "../common/protocol.h"
#endif

struct available_matches {
    struct Server_BroadcastMatch *broad;
    struct available_matches *next;
};

extern int player_id;

void handle_packet(int client, struct Packet *packet);
struct Server_BroadcastMatch *find_node(struct available_matches *head, int match);
void print_available_matches();
void create_match(int sockfd);
void join_match(int sockfd, int match);
void make_move(int sockfd, int moveX, int moveY, int match);
void send_play_again(int sockfd, int choice, int match);
void send_winner_play_again(int sockfd, int choice, int match);
void clear_grid(char grid[3][3]);
void clear_game_vars();
void new_match_game_vars();