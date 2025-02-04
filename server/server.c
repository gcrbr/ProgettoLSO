#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

#define PORT 8080

void init_socket() {
    int sockfd = 0;
    int opt = 0;
    struct sockaddr_in address;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("[ERRORE] Impossibile inizializzare socket");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("[ERRORE] Impossibile configurare socket");
        exit(1);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("[ERRORE] Impossibile eseguire il binding");
        exit(1);
    }

    if(listen(sockfd, 3) < 0) {
        perror("[ERRORE] Impossibile avviare il listening");
        exit(1);
    }

    /*
        Continuare con ACCEPT e poi THREADING
    */
}

int main() {
    printf("TRIS server\n");
    init_socket();
    return 0;
}