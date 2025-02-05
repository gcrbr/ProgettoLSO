#ifndef STRUCTURES_H
#define STRUCTURES_H
#include "structures.h"
#endif

#include <stdio.h>

short curr_clients_size = 0;

// Parametro excxept serve ad escludere qualcuno, utile se non si vuole broadcast allo stesso giocatore che manda il broadcast
void broadcast_packet(struct client_node *head, struct Packet *packet, int except) {
    struct client_node *current = head;
    while(current != NULL) {
        if(current->val->conn != except) {
            send_packet(current->val->conn, packet);
        }
        current = current->next;
    }
}