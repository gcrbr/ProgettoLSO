#include <stdlib.h>
#include <netinet/in.h>

#ifndef STRUCTURES_H
#define STRUCTURES_H
#include "structures.h"
#endif

void initialize_list(struct client_node **head, struct client *client) {
    *head = malloc(sizeof(struct client_node));
    if (*head == NULL) {
        return;
    }
    (*head)->val = client;
    (*head)->next = NULL;
}

void add_client(struct client_node *head, struct client *client) {
    if(head == NULL) {
        initialize_list(&head, client);
    }
    struct client_node *new_node = malloc(sizeof(struct client_node));
    if (new_node == NULL) {
        return;
    }
    new_node->val = client;
    new_node->next = NULL;

    struct client_node *curr = head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = new_node;
}

void remove_client(struct client_node **head, struct client *client_to_remove) {
    if (head == NULL || *head == NULL) {
        return;
    }
    struct client_node *curr = *head;
    struct client_node *prev = NULL;
    while (curr != NULL) {
        if (curr->val == client_to_remove) {
            if (prev == NULL) {
                *head = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr->val);
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}