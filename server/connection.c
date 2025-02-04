#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#ifndef CONNECTION_H
#define CONNECTION_H
#include "connection.h"
#endif

#ifndef MODELS_H
#define MODELS_H
#include "../common/models.h"
#endif

#define BUFFER_SIZE 1024

void *joiner_thread(void *args) {
    struct joiner_thread_args *thread_args = (struct joiner_thread_args*)args;
    pthread_join((pthread_t)(thread_args->thread), NULL);
    printf("%s Connessione chiusa (fd=%d)\n", MSG_INFO, thread_args->client->conn);
    remove_client(&clients, thread_args->client);
    free(thread_args->client);
    return NULL;
} 

void *server_thread(void *args) {
    struct client *client = (struct client *)args;
    char buf[BUFFER_SIZE];
    while(1) {
        if(recv(client->conn, buf, BUFFER_SIZE, 0) == 0) {
            break;
        }
    }
    return NULL;
}