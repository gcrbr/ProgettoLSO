#ifndef STRUCTURES_H
#define STRUCTURES_H
#include "structures.h"
#endif

#ifndef CONNECTION_H
#define CONNECTION_H
#include "connection.h"
#endif

#ifndef MODELS_H
#define MODELS_H
#include "../common/models.h"
#endif

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 255

extern struct client_node *clients;

void init_socket() {
    int sockfd = 0;
    int opt = 0;
    struct sockaddr_in address;
    int conn = 0;
    socklen_t addrlen;
    pthread_t threads[MAX_CLIENTS];
    int thread_count = 0;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "%s Impossibile inizializzare socket: %s\n", MSG_ERROR, strerror(errno));
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        fprintf(stderr, "%s Impossibile configurare socket: %s\n", MSG_ERROR, strerror(errno));
        exit(1);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        fprintf(stderr, "%s Impossibile eseguire funzione bind: %s\n", MSG_ERROR, strerror(errno));
        exit(1);
    }

    if(listen(sockfd, 3) < 0) {
        fprintf(stderr, "%s Impossibile eseguire funzione listen: %s\n", MSG_ERROR, strerror(errno));
        exit(1);
    }

    printf("%s In ascolto...\n", MSG_INFO);

    while(1) {
        if((conn = accept(sockfd, (struct sockaddr*)&address, &addrlen)) < 0) {
            fprintf(stderr, "%s Impossibile accettare la connessione: %s\n", MSG_ERROR, strerror(errno));
        }else {
            if(curr_clients_size < MAX_CLIENTS) {
                printf("%s Connessione accettata (fd=%d)\n", MSG_INFO, conn);

                struct client *new_client = malloc(sizeof(struct client));
                new_client->conn = conn;
                new_client->addr = address;

                struct Player *player = malloc(sizeof(struct Player));
                player->id = -1;
                new_client->player = player;
                add_node((struct generic_node **)&clients, (void *)new_client);

                if(pthread_create(&threads[thread_count++], NULL, server_thread, (void *)new_client) < 0) {
                    fprintf(stderr, "%s Impossibile avviare thread: %s\n", MSG_ERROR, strerror(errno));
                    close(conn);
                    remove_node((struct generic_node **)&clients, (void *)new_client);
                    free(new_client);
                }

                struct joiner_thread_args *thread_args = malloc(sizeof(struct joiner_thread_args));
                thread_args->client = new_client;
                thread_args->thread = threads[thread_count - 1];

                pthread_t joiner_thread_id;
                if(pthread_create(&joiner_thread_id, NULL, joiner_thread, (void *)thread_args) < 0) {
                    fprintf(stderr, "%s Impossibile avviare joiner thread: %s\n", MSG_ERROR, strerror(errno));
                    close(conn);
                    remove_node((struct generic_node **)&clients, (void *)new_client);
                    free(new_client);
                }
            }else {
                close(conn);
                printf("%s Impossibile accettare nuova connessione. Limite massimo raggiunto (%d)\n", MSG_WARNING, curr_clients_size);
            }
        }
    }

    printf("%s Socket del server chiuso\n", MSG_INFO);
    close(sockfd);
}

int main() {
    printf("Tris Server\n\n");
    printf("%s Utilizzando la porta %d\n", MSG_INFO, PORT);
    init_socket();
    return 0;
}