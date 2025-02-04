#include <netinet/in.h>

struct client {
    int conn;
    struct sockaddr_in addr;
};

struct client_node {
    struct client *val;
    struct client_node *next;
};

void initialize_list(struct client_node **head, struct client *client);
void add_client(struct client_node *head, struct client *client);
void remove_client(struct client_node **head, struct client *client_to_remove);