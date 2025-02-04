#ifndef MODELS_H
#define MODELS_H
#include "models.h"
#endif

#include <stdlib.h>
#include <stdio.h>

struct MatchList *matches = NULL;

void initialize_list(struct generic_node **head, void *node) {
    *head = malloc(sizeof(struct generic_node));
    if (*head == NULL) {
        return;
    }
    (*head)->val = node;
    (*head)->next = NULL;
}

void add_node(struct generic_node **head, void *node) {
    if(*head == NULL) {
        initialize_list(head, node);
        return;
    }

    struct generic_node *new_node = malloc(sizeof(struct generic_node));
    if(new_node == NULL) {
        return;
    }
    new_node->val = node;
    new_node->next = NULL;

    struct generic_node *curr = *head;
    while(curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = new_node;
}

void remove_node(struct generic_node **head, void *node_to_remove) {
    if(head == NULL || *head == NULL) {
        return;
    }
    struct generic_node *curr = *head;
    struct generic_node *prev = NULL;
    while(curr != NULL) {
        if(curr->val == node_to_remove) {
            if (prev == NULL) {
                *head = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void remove_matches_by_player(struct MatchList **head, int player_id) {
    if(head == NULL || *head == NULL) {
        return;
    }

    struct MatchList *curr = *head;
    struct MatchList *prev = NULL;

    while(curr != NULL) {
        if(curr->val->participants[0]->id == player_id) {
            if(prev == NULL) {
                *head = curr->next;
            }else {
                prev->next = curr->next;
            }
            free(curr);
            curr = (prev == NULL) ? *head : prev->next;
        }else {
            prev = curr;
            curr = curr->next;
        }
    }
}

struct Match *get_match_by_id(struct MatchList *head, int id) {
    struct MatchList *current = head;
    int index = 0;
    while(current != NULL) {
        if(index == id) {
            return current->val;
        }
        current = current->next;
        index++;
    }
    return NULL;
}
