#ifndef MODELS_H
#define MODELS_H
#include "../common/models.h"
#endif

#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "../common/protocol.h"
#endif

#include <netinet/in.h>

struct client {
    int conn;
    struct sockaddr_in addr;
    struct Player *player;
};

struct client_node {
    struct client *val;
    struct client_node *next;
};

extern short curr_clients_size;

void broadcast_packet(struct client_node *head, struct Packet *packet, int except);