#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "image_tagger.h"
#include "send_page.h"
#include "play_game.h"

void make_guess(char guess[MAX_GUESS_LEN], player * player) {
  char * all_guesses;
  int i;

  // add keyword to player's guesses
  strcpy(player->guesses[player->guesses_made], guess);
  player->guesses_made += 1;

  // create string from all guesses
  all_guesses = malloc((player->guesses_made) * sizeof(char[MAX_GUESS_LEN]));
  all_guesses[0] = '\0';
  for (i=0; i<player->guesses_made; i++)
  {
    if (i>0)
    {
      strcat(all_guesses, ", ");
    }
    strcat(all_guesses, player->guesses[i]);
  }

  // send accepted page with all guesses
  send_page(ACCEPTED, player->player_socket, all_guesses);

  free(all_guesses);
}
