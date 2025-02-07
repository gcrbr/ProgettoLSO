#ifndef GUI_H
#define GUI_H
#include "gui.h"
#endif

#ifndef MODELS_H
#define MODELS_H
#include "../common/models.h"
#endif

#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "../common/protocol.h"
#endif

#ifndef CONNECTION_H
#define CONNECTION_H
#include "connection.h"
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

#define DEFAULT_IP      "127.0.0.1"
#define DEFAULT_PORT    8080

#define BUFFER_SIZE     1024

void *client_thread(void *_sockfd) {
    int sockfd = *((int*)_sockfd);
    char buffer[BUFFER_SIZE];
    size_t total = 0;
    while(1) {
        ssize_t received = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if(received <= 0) {
            printf("%s Connessione chiusa dal server\n", MSG_INFO);
            exit(0);
            break;
        }
        total = received;
        while(total > 0) {
            char *block = buffer + (received - total);
            struct Packet *packet = malloc(sizeof(struct Packet));
            packet->id = block[0];
            packet->size = block[1] + (block[2] << 8);
            packet->content = malloc(sizeof(char) * packet->size);
            memcpy(packet->content, block + 3, packet->size);
            handle_packet(sockfd, packet);
            total -= packet->size + 3;
            free(packet->content);
            free(packet);
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    char *ip = DEFAULT_IP;
    int port = DEFAULT_PORT;
    int sockfd = 0;
    struct sockaddr_in address;

    pthread_t client_t;
    pthread_t ui_t;

    clear_screen();

    if(argc == 3) { // Sono stati passati IP e porta manualmente
        ip = argv[1];
        if((sscanf(argv[2], "%d", &port)) <= 0 || port < 0 || port > 65535) {
            fprintf(stderr, "%s È stata fornita una porta non valida come parametro (%s)\n", MSG_ERROR, argv[2]);
            return 1;
        }
    }

    printf("%s Connessione al server %s:%d\n", MSG_INFO, ip, port);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "%s Impossibile inizializzare socket: %s\n", MSG_ERROR, strerror(errno));
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if(inet_pton(AF_INET, ip, &address.sin_addr) <= 0) {
        fprintf(stderr, "%s L'indirizzo %s non è valido: %s\n", MSG_ERROR, ip, strerror(errno));
        return 1;
    }

    if(connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        fprintf(stderr, "%s Impossibile connettersi al server: %s\n", MSG_ERROR, strerror(errno));
        return 1;
    }

    struct Packet *packet = malloc(sizeof(struct Packet));
    packet->id = CLIENT_HANDSHAKE;
    send_packet(sockfd, packet);
    free(packet);

    if(pthread_create(&client_t, NULL, client_thread, &sockfd) < 0) {
        fprintf(stderr, "%s Impossibile avviare client thread: %s\n", MSG_ERROR, strerror(errno));
        return 1;
    }

    if(pthread_create(&ui_t, NULL, ui_thread, &sockfd) < 0) {
        fprintf(stderr, "%s Impossibile avviare ui thread: %s\n", MSG_ERROR, strerror(errno));
        return 1;
    }
    
    pthread_join(client_t, NULL);
    pthread_join(ui_t, NULL);

    return 0;
}