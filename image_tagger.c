#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#include "send_page.h"

#define MAX_BUFFER 2049
#define MAX_URL_SIZE 256

typedef enum
{
  INITIALISING,
  WAITING_FOR_PLAYERS,
  PLAYING_GAME
} SERVER_STATE;

typedef enum
{
    GET,
    POST,
    UNKNOWN
} METHOD;

struct http_req_header
{
  METHOD method;
  char url[MAX_URL_SIZE];
  char protocol[MAX_URL_SIZE];
};

void wait_for_players(SERVER_STATE * state, int welcome_socket);
bool handle_http_request(int socket, SERVER_STATE * state);
struct http_req_header parse_req_header(char * req_header_raw);
METHOD parse_method(char * method_raw);

int main(int argc, char *argv[])
{
  SERVER_STATE state = INITIALISING;

  int server_ip;
  int server_port;

  int welcome_socket;

  struct sockaddr_in serv_addr;

  // setup server ip and port number
  if (argc < 3)
  {
    perror("Please specify server IP and port number");
    exit(EXIT_FAILURE);
  } else {
    server_ip = inet_addr(argv[1]);
    server_port = htons(atoi(argv[2]));
  }

  // create IPV4 TCP welcome socket
  welcome_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (welcome_socket < 0) {
    perror("Failed to create welcome socket");
    exit(EXIT_FAILURE);
  }

  // reuse the welcome socket
  int const reuse = 1;
  if (setsockopt(welcome_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
  {
    perror("Faile to create welcome socket");
    exit(EXIT_FAILURE);
  }

  // setup serv_addr
  memset((char *)&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = server_ip;
  serv_addr.sin_port = server_port;

  // bind the socket
  if (bind(welcome_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("Failed to bind socket");
    exit(EXIT_FAILURE);
  }

  // listen on socket
  if (listen(welcome_socket, 2) < 0)
  {
    perror("Failed to listen on socket");
    exit(EXIT_FAILURE);
  }

  printf("image_tagger server is now running at IP: %s on port %s\n",
    argv[1], argv[2]);

  state = WAITING_FOR_PLAYERS;
  wait_for_players(&state, welcome_socket);

  return 0;
}

void wait_for_players(SERVER_STATE * state, int welcome_socket)
{
  // setup client sockets
  fd_set client_sockets;
  FD_ZERO(&client_sockets);
  FD_SET(welcome_socket, &client_sockets);
  int socket_max = welcome_socket;


  while (*state == WAITING_FOR_PLAYERS)
  {
    // monitor file descriptors
    fd_set readfds = client_sockets;
    if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0)
    {
      perror("select");
      exit(EXIT_FAILURE);
    }

    // loop all possible descriptor
    for (int i = 0; i <= socket_max; ++i)
    {
      // determine if the current file descriptor is active
      if (FD_ISSET(i, &readfds))
      {
        // create new socket if there is new incoming connection request
        if (i == welcome_socket)
        {
          struct sockaddr_in cliaddr;
          socklen_t clilen = sizeof(cliaddr);
          int newsockfd = accept(welcome_socket, (struct sockaddr *)&cliaddr, &clilen);
          if (newsockfd < 0)
          perror("accept");
          else
          {
            // add the socket to the set
            FD_SET(newsockfd, &client_sockets);
            // update the maximum tracker
            if (newsockfd > socket_max)
            socket_max = newsockfd;
            // print out the IP and the socket number
            char ip[INET_ADDRSTRLEN];
            printf(
              "New connection from %s on socket %d\n",
              // convert to human readable string
              inet_ntop(cliaddr.sin_family, &cliaddr.sin_addr, ip, INET_ADDRSTRLEN),
              newsockfd
            );
          }
        }
        // a request is sent from the client
        else if (!handle_http_request(i, state))
        {
          close(i);
          FD_CLR(i, &client_sockets);
        }
      }
    }
  }
}

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
