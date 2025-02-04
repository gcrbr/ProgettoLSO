#define MSG_INFO    "[\x1b[34mINFO\x1b[0m]"
#define MSG_WARNING "[\x1b[33mWARNING\x1b[0m]"
#define MSG_ERROR   "[\x1b[31mERROR\x1b[0m]"
#define MSG_DEBUG   "[\x1b[32mDEBUG\x1b[0m]"

#define DEBUG   1

struct Player {
    int id;
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

struct Match {
    struct Player *participants[2];
    struct Player *requester;
    char grid[3][3];
    int state;
};

struct MatchList {
    struct Match *val;
    struct MatchList *next;
};

extern struct MatchList *matches;

struct generic_node {
    void *val;
    struct generic_node *next;
};

void initialize_list(struct generic_node **head, void *node);
void add_node(struct generic_node **head, void *node);
void remove_node(struct generic_node **head, void *node_to_remove);
void remove_matches_by_player(struct MatchList **head, int player_id);
struct Match *get_match_by_id(struct MatchList *head, int id);