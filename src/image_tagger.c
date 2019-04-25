#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#include "image_tagger.h"
#include "handle_http_request.h"

int main(int argc, char *argv[])
{
  server_state state = INITIALISING;

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
  listen_for_reqs(&state, welcome_socket);

  return 0;
}

void listen_for_reqs(server_state * state, int welcome_socket)
{
  // setup client sockets
  fd_set client_sockets;
  FD_ZERO(&client_sockets);
  FD_SET(welcome_socket, &client_sockets);
  int socket_max = welcome_socket;

  // initialise players
  player p1;
  player p2;
  int player_count = 0;

  while (1)
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
        else if (!handle_http_request(i, state, &p1, &p2, &player_count))
        {
          close(i);
          FD_CLR(i, &client_sockets);
        }
      }
    }
  }
}
