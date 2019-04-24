#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


#define MAX_BUFFER 2048

int main(int argc, char *argv[])
{
  int server_ip;
  int server_port;

  int socketfd;
  int new_socketfd;

  struct sockaddr_in serv_addr;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  char buffer[MAX_BUFFER] = {0};
  int bytes_read;
  int bytes_written;

  // setup server ip and port number
  if (argc < 2)
  {
    perror("Please specify server IP and port number");
    exit(EXIT_FAILURE);
  } else {
    server_ip = inet_addr(argv[1]);
    server_port = htons(atoi(argv[2]));
  }

  // create IPV4 TCP socket
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    perror("Failed to create socket");
    exit(EXIT_FAILURE);
  }

  // setup serv_addr
  memset((char *)&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = server_ip;
  serv_addr.sin_port = server_port;

  // bind the socket
  if (bind(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("Failed to bind socket");
    exit(EXIT_FAILURE);
  }

  // listen on socket
  if (listen(socketfd, 2) < 0)
  {
    perror("Failed to listen on socket");
    exit(EXIT_FAILURE);
  }

  // accept a new connection
  if ((new_socketfd = accept(socketfd, (struct sockaddr*) &client_addr,
    &client_addr_len)) < 0)
  {
    perror("Failed to accept connection");
    exit(EXIT_FAILURE);
  }

  // read message from socket
  bytes_read = read(new_socketfd, buffer, MAX_BUFFER);
  if (bytes_read < 0)
  {
    perror("No bytes read");
    exit(EXIT_FAILURE);
  }

  // print the received message
  printf("Message received: %s\n", buffer);

  // write back to client
  char *return_message = "I received you message";
  bytes_written = write(new_socketfd, return_message, strlen(return_message));
  if (bytes_written < 0)
  {
    perror("Failed to write to socket");
    exit(EXIT_FAILURE);
  }

  close(new_socketfd);
}
