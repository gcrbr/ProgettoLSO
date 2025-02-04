#ifndef STRUCTURES_H
#define STRUCTURES_H
#include "structures.h"
#endif

struct joiner_thread_args {
    struct client *client;
    pthread_t thread;
};

struct client_node *clients;

void *joiner_thread(void *args);
void *server_thread(void *args);