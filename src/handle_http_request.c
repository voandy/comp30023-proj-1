#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>

#include "image_tagger.h"
#include "send_page.h"
#include "handle_http_request.h"

static void init_player(player * player, struct http_req_header req_header,
  int player_count);

// handles http request from a client socket
bool handle_http_request(int socket, server_state * state,
  player * p1, player * p2, int * player_count)
{
  char buffer[MAX_BUFFER] = {0};
  int bytes_read;
  char * req_header_raw;
  struct http_req_header req_header;

  // read request header
  bytes_read = read(socket, buffer, MAX_BUFFER);
  if (bytes_read <= 0)
  {
    if (bytes_read < 0)
    {
      perror("Failed to read from socket");
      exit(EXIT_FAILURE);
    } else {
      printf("Socket %d closed the connection\n", socket);
      return false;
    }
  }

  // add a null byte to signify the end of the string
  buffer[bytes_read] = 0;

  // parse the request header into a http_req_header
  req_header_raw = buffer;
  req_header = parse_req_header(req_header_raw);

  // vary based on game state
  switch(*state){
    case WAITING_FOR_PLAYERS:
      switch(req_header.method)
      {
        case GET:
          if(!send_page(INTRO, socket, NULL))
          {
            return false;
          }
          break;
        case POST:
          if (!send_page(START, socket, req_header.user))
          {
            return false;
          }
          // initialise players until there are 2 players in the game
          switch(*player_count)
          {
            case 0:
              init_player(p1, req_header, *player_count);
              *player_count += 1;
              break;
            case 1:
              init_player(p2, req_header, *player_count);
              *player_count += 1;
              *state = READY_TO_PLAY;
              break;
          }
          break;
        default:
          printf("Invalid request");
          return false;
      }
    case READY_TO_PLAY:
      break;
    case PLAYING_GAME:
      break;
    default:
      printf("Invalid request");
      return false;
  }


  return true;
}

// initialise players
static void init_player(player * player, struct http_req_header req_header,
  int player_count)
{
 player -> player_no = player_count;
 strcpy(player -> player_name, req_header.user);
 player -> guesses_made = 0;
}

// given a string of raw data returns a http_req_header
struct http_req_header parse_req_header(char * req_header_raw)
{
  struct http_req_header req_header = {0};
  char * result;

  // get start status
  result = strstr(req_header_raw, "start=");
  if (result)
  {
    req_header.start = true;
  }

  // get quit status
  result = strstr(req_header_raw, "quit=");
  if (result)
  {
    req_header.quit = true;
  }

  // get guess status
  result = strstr(req_header_raw, "guess=");
  if (result)
  {
    req_header.guess = true;
  }

  // get user
  result = strstr(req_header_raw, "user=");
  if (result) {
    result += 5;
    strcpy(req_header.user, result);
  }

  // get keyword
  result = strstr(req_header_raw, "keyword=");
  if (result)
  {
    result+= 8;

    // remove text after keyword
    char delim[] = "&";
    result = strtok(result, delim);

    strcpy(req_header.keyword, result);
  }

  // setup strtok
  char * token;
  char delim[] = " \n";
  token = strtok(req_header_raw, delim);

  // get method
  req_header.method = parse_method(token);
  token = strtok(0, delim);

  // get url
  strcpy(req_header.url, token);
  token = strtok(0, delim);

  // get protocol
  strcpy(req_header.protocol, token);

  return req_header;
}

// given a string returns the associated method
METHOD parse_method(char * method_raw)
{
  METHOD method = UNKNOWN;
  if (strcmp(method_raw, "GET") == 0)
  {
    method = GET;
  }
  else if (strcmp(method_raw, "POST") == 0)
  {
    method = POST;
  }
  return method;
}
