#define MSG_INFO    "[\x1b[34mINFO\x1b[0m]"
#define MSG_WARNING "[\x1b[33mWARNING\x1b[0m]"
#define MSG_ERROR   "[\x1b[31mERROR\x1b[0m]"
#define MSG_DEBUG   "[\x1b[32mDEBUG\x1b[0m]"

#define DEBUG   1

struct Player {
    int id;
    int busy;   // 1 Se è già occupato in una partita, 0 Altrimenti
};

#define STATE_TERMINATED    0
#define STATE_INPROGRESS    1
#define STATE_WAITING       2 // Probabilmente inutile (usiamo stato dei turni(player1/player2))
#define STATE_CREATED       3

#define STATE_TURN_PLAYER1  4
#define STATE_TURN_PLAYER2  5

#define STATE_WIN           6
#define STATE_LOSE          7
#define STATE_DRAW          8

#define MAX_MATCHES         255

struct RequestNode {
    struct Player *requester;
    struct RequestNode *next;
};



struct Match {
    struct Player *participants[2];
    struct RequestNode *requests_head; // Testa della coda delle richieste
    struct RequestNode *requests_tail; // Coda della coda delle richieste

    char grid[3][3];        // 1 = X, 2 = Cerchio
    int free_slots;          // Parte da 9, quando si raggiunge 0 e non ha ancora vinto nessuno è pareggio
    int state;
    
    int play_again_counter;         // Conteggio dei voti per rigiocare, 0=Nessuno, 1=Uno ha detto sì, 2=Anche l'altro ha detto sì
    struct Player *play_again[2];   // Array dei giocatori che hanno detto sì
};

struct MatchList {
    struct Match *val;
    struct MatchList *next;
};

extern struct MatchList *matches;
extern short curr_matches_size;

struct generic_node {
    void *val;
    struct generic_node *next;
};

void initialize_list(struct generic_node **head, void *node);
void add_node(struct generic_node **head, void *node);
void remove_node(struct generic_node **head, void *node_to_remove);
void remove_matches_by_player(struct MatchList **head, int player_id);
struct Match *get_match_by_id(struct MatchList *head, int id);
void add_requester(struct Match* match, struct RequestNode* node);
void delete_from_head(struct Match* match);