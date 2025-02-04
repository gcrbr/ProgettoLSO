#include <netinet/in.h>

struct client {
    int conn;
    struct sockaddr_in addr;
};

struct client_node {
    struct client *val;
    struct client_node *next;
};