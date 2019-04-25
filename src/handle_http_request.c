#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>

#include "image_tagger.h"
#include "send_page.h"
#include "handle_http_request.h"

// handles http request from a client socket
bool handle_http_request(int socket, SERVER_STATE * state)
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
      if (!send_page(INTRO, socket))
      {
        return false;
      }
      break;
    case POST:
      printf("%s\n", "Received post request");
      break;
    default:
      printf("%s\n", "Invalid request");
  }

  return true;
}

// given a string of raw data returns a http_req_header
struct http_req_header parse_req_header(char * req_header_raw)
{
  struct http_req_header req_header = {0};
  char * token;
  char delim[] = " \n";

  token = strtok(req_header_raw, delim);

  req_header.method = parse_method(token);
  token = strtok(0, delim);

  strcpy(req_header.url, token);
  token = strtok(0, delim);

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
