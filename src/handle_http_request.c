#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>

#include "image_tagger.h"
#include "send_page.h"
#include "handle_http_request.h"

// handles http request from a client socket
bool handle_http_request(int socket, server_state * state,
  player * p1, player * p2, int * player_count)
{
  printf("new http req\n");
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

  switch(req_header.method)
  {
    case GET:
      if (!send_page(FIRST_TURN, socket))
      {
        return false;
      }
      break;
    case POST:
      break;
    default:
      printf("%s\n", "Invalid request");
      return false;
  }

  return true;
}



// given a string of raw data returns a http_req_header
struct http_req_header parse_req_header(char * req_header_raw)
{
  struct http_req_header req_header = {0};
  char * result;

  printf("%s\n", req_header_raw);

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
