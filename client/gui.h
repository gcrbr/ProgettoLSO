extern int is_in_game_menu;
extern int accepted;
extern int game_owned;
extern int game_running;
extern int curr_match_id;
extern int game_state;
extern int other_player_play_again;
extern char grid[3][3];

void clear_screen();
void *ui_thread();