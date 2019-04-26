#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "image_tagger.h"
#include "send_page.h"
#include "play_game.h"

// returns true if player has won the game else returns false
bool make_guess(char guess[MAX_GUESS_LEN], player * curr_player,
    player * opponent) {
  char * all_guesses;
  int i;

  // compare current guess to guesses made by opponent
  for (i=0; i<opponent->guesses_made; i++)
  {
    if (strcmp(guess, opponent->guesses[i]) == 0)
    {
      // player has guessed one of opponent's words and won
      send_page(ENDGAME, curr_player->player_socket, NULL);
      return true;
    }
  }

  // add keyword to player's guesses
  strcpy(curr_player->guesses[curr_player->guesses_made], guess);
  curr_player->guesses_made += 1;

  // create string from all guesses
  all_guesses = malloc((curr_player->guesses_made) * sizeof(char[MAX_GUESS_LEN]));
  all_guesses[0] = '\0';
  for (i=0; i<curr_player->guesses_made; i++)
  {
    if (i>0)
    {
      strcat(all_guesses, ", ");
    }
    strcat(all_guesses, curr_player->guesses[i]);
  }

  // send accepted page with all guesses
  send_page(ACCEPTED, curr_player->player_socket, all_guesses);

  free(all_guesses);
  return false;
}
