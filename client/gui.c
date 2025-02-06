#ifndef CONNECTION_H
#define CONNECTION_H
#include "connection.h"
#endif

#ifndef MODELS_H
#define MODELS_H
#include "../common/models.h"
#endif

#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "../common/models.h"
#endif

#include <stdio.h>

int is_in_game_menu = 0;
int accepted = -1;
int game_owned = 0;
int game_running = 0;
int curr_match_id = -1;
int game_state = -1;
int other_player_play_again = -1;
char grid[3][3];

void clear_screen() {
    printf("\e[1;1H\e[2J");
}

void print_grid() {
    char symbols[3] = {'-', 'X', 'O'};
    printf("\n%c %c %c\n%c %c %c\n%c %c %c\n\n", 
        symbols[grid[0][0]], symbols[grid[1][0]], symbols[grid[2][0]],
        symbols[grid[0][1]], symbols[grid[1][1]], symbols[grid[2][1]],
        symbols[grid[0][2]], symbols[grid[1][2]], symbols[grid[2][2]]
    );
}

void game_ui(int sockfd) {
    is_in_game_menu = 1;
    clear_screen();
    printf("MENU DI GIOCO\n");
    printf("Premi Q per uscire\n\n");
    while(1) {
        if(game_running && game_state != -1) {
            if(game_state == STATE_WIN) {
                print_grid();
                printf("\nComplimenti, hai vinto!\n");
            }else if(game_state == STATE_LOSE) {
                print_grid();
                printf("\nOh no, hai perso...\n");
            }else if(game_state == STATE_DRAW) {
                print_grid();
                printf("\nPareggio!\n");
            }

            if(game_state == STATE_WIN || game_state == STATE_LOSE || game_state == STATE_DRAW) {
                game_state = -1;
                char play_again = 0;
                clear_grid(grid);
                do {
                    printf("\nVuoi giocare ancora? (S/N): ");
                    scanf(" %c", &play_again);
                } while(play_again != 's' && play_again != 'S' && play_again != 'n' && play_again != 'N');
                send_play_again(sockfd, play_again=='s'||play_again=='S', curr_match_id);

                if(other_player_play_again == 0 || play_again == 'n' || play_again == 'N') {
                    clear_screen();
                    if(other_player_play_again == 0) {
                        printf("Il giocatore ha rifiutato la rivincita\n\n");
                    }
                    clear_game_vars();
                    break;
                }
            }

            if((game_state == STATE_TURN_PLAYER1 && game_owned) || (game_state == STATE_TURN_PLAYER2 && !game_owned)) {
                game_state = -1;
                print_grid();
                int mossaX;
                int mossaY;
                printf("È il tuo turno\n");

                do {
                    printf("Mossa (Riga,Colonna): ");
                } while (scanf("%d,%d", &mossaY, &mossaX) != 2 ||
                    mossaX < 1 || mossaX > 3 ||
                    mossaY < 1 || mossaY > 3 ||
                    grid[mossaX - 1][mossaY - 1] != 0);
                
                grid[mossaX - 1][mossaY - 1] = game_owned ? 1 : 2;
                make_move(sockfd, mossaX-1, mossaY-1, curr_match_id);

                print_grid();
            }else if((game_state == STATE_TURN_PLAYER1 && !game_owned) || (game_state == STATE_TURN_PLAYER2 && game_owned)){
                printf("È il turno dell'altro giocatore\n\n");
            }
        }
    }
    is_in_game_menu = 0;
}

void *ui_thread(void *_sockfd) {
    int sockfd = *((int*)_sockfd);
    int scelta = 0;
    while(1) {
        if(player_id != -1) {
            printf("MENU\n");
            printf("1.  Crea una nuova partita\n");
            printf("2.  Visualizza partite disponibili\n");
            printf("3.  Unisciti ad una partita\n");
            printf("> Scegli opzione: ");
            if(scanf("%d", &scelta) < 0 || scelta < 1 || scelta > 3) {
                fprintf(stderr, "%s Valore non valido fornito come scelta\n", MSG_ERROR);
            }

            if(scelta == 1) {
                scelta = -1;
                printf("%s Creo partita...\n", MSG_INFO);
                game_owned = 1;
                create_match(sockfd);
                game_ui(sockfd);
            }

            if(scelta==2){
                scelta=-1;
                printf("%s Partite disponibili \n",MSG_INFO);
                print_available_matches();
            }

            if(scelta == 3) {
                scelta = -1;
                int id_partita = -1;
                do {
                    printf("> Inserisci numero partita: ");
                } while(scanf("%d", &id_partita) < 0);
                join_match(sockfd, id_partita);
                printf("%s Inviata richiesta di accesso, attendendo risposta...\n\n", MSG_INFO);
                while(accepted == -1) {}
                if(accepted) {
                    printf("ha accetttt");
                    accepted = -1;
                    game_ui(sockfd);
                }
                accepted = -1;
            }
        }
    }
    return NULL;
}