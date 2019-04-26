#ifndef IMAGE_TAGGER_H
#define IMAGE_TAGGER_H

#define MAX_BUFFER 2049

#define MAX_NAME_LEN 128
#define MAX_GUESSES 512
#define MAX_GUESS_LEN 128

typedef enum
{
  INITIALISING,
  WAITING_FOR_PLAYERS,
  PLAYING_GAME,
  GAME_OVER
} server_state;

typedef struct
{
  int player_socket;
  char player_name[MAX_NAME_LEN];
  bool ready;
  char guesses[MAX_GUESSES][MAX_GUESS_LEN];
  int guesses_made;
}player;

void listen_for_reqs(server_state * state, int welcome_socket);

#endif
