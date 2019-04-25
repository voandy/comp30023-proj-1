#ifndef IMAGE_TAGGER_H
#define IMAGE_TAGGER_H

#define MAX_BUFFER 2049

typedef enum
{
  INITIALISING,
  WAITING_FOR_PLAYERS,
  PLAYING_GAME
} SERVER_STATE;

void listen_for_reqs(SERVER_STATE * state, int welcome_socket);

#endif
