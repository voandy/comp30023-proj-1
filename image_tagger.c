#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>

#define MAX_BUFFER 2049
#define MAX_URL_SIZE 256

#define PATH_WELCOME "html_files/1_welcome.html"
#define PATH_START "html_files/2_start.html"
#define PATH_FIRST_TURN "html_files/3_first_turn.html"
#define PATH_ACCEPTED "html_files/4_accepted.html"
#define PATH_DISCARDED "html_files/5_discarded.html"
#define PATH_ENDGAME "html_files/6_endgame.html"
#define PATH_GAMEOVER "html_files/7_gameover.html"

#define HTTP_200_FORMAT "HTTP/1.1 200 OK\r\n\
                        Content-Type: text/html\r\n\
                        Content-Length: %ld\r\n\r\n"

typedef enum
{
    GET,
    POST,
    UNKNOWN
} METHOD;

typedef enum
{
  WELCOME,
  START,
  FIRST_TURN,
  ACCEPTED,
  DISCARDED,
  ENDGAME,
  GAMEOVER
} PAGE_TYPE;

struct http_req_header
{
  METHOD method;
  char url[MAX_URL_SIZE];
  char protocol[MAX_URL_SIZE];
};

static bool handle_http_request(int socketfd);
static struct http_req_header parse_req_header(char * req_header_raw);
static METHOD parse_method(char * method_raw);
static bool send_page(PAGE_TYPE page_type, int socketfd);

int main(int argc, char *argv[])
{
  int server_ip;
  int server_port;

  int socketfd;
  int new_socketfd;

  struct sockaddr_in serv_addr;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

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

  printf("image_tagger server is now running at IP: %s on port %s\n", argv[1], argv[2]);

  // accept a new connection
  if ((new_socketfd = accept(socketfd, (struct sockaddr*) &client_addr,
    &client_addr_len)) < 0)
  {
    perror("Failed to accept connection");
    exit(EXIT_FAILURE);
  }

  handle_http_request(new_socketfd);

  close(new_socketfd);
}

// handles http request from a client socket
static bool handle_http_request(int socketfd)
{
  char buffer[MAX_BUFFER] = {0};
  int bytes_read;
  char * req_header_raw;
  struct http_req_header req_header;

  // read request header
  bytes_read = read(socketfd, buffer, MAX_BUFFER);
  if (bytes_read <= 0)
  {
    perror("Failed to read from socket");
    exit(EXIT_FAILURE);
  }

  // add a null byte to signify the end of the string
  buffer[bytes_read] = 0;

  // parse the request header into a http_req_header
  req_header_raw = buffer;
  req_header = parse_req_header(req_header_raw);

  switch(req_header.method)
  {
    case GET:
      printf("%s\n", "Received get request");
      send_page(WELCOME, socketfd);
      break;
    case POST:
      printf("%s\n", "Received post request");
      break;
    default:
      printf("%s\n", "Invalid request");
  }

  printf("%s\n", buffer);

  return true;
}

// given a string of raw data returns a http_req_header
static struct http_req_header parse_req_header(char * req_header_raw)
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
static METHOD parse_method(char * method_raw)
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

static bool send_page(PAGE_TYPE page_type, int socketfd)
{
  char * path;
  struct stat file_stats;
  char buffer[MAX_BUFFER] = {0};
  int size_content;
  int filefd;

  switch(page_type){
    case WELCOME: path = PATH_WELCOME;
      break;
    case START: path = PATH_START;
      break;
    case FIRST_TURN: path = PATH_FIRST_TURN;
      break;
    case ACCEPTED: path = PATH_ACCEPTED;
      break;
    case DISCARDED: path = PATH_DISCARDED;
      break;
    case ENDGAME: path = PATH_ENDGAME;
      break;
    case GAMEOVER: path = PATH_GAMEOVER;
      break;
    default:
      printf("Invalid page");
      return false;
  }

  // get stats if html file
  stat(path, &file_stats);

  // compose http header
  size_content = sprintf(buffer, HTTP_200_FORMAT, file_stats.st_size);

  // send http header
  if (write(socketfd, buffer, size_content) < 0)
  {
    printf("Failed to send header");
    return false;
  }

  // send the html file
  filefd = open(path, O_RDONLY);
  do
  {
    size_content = sendfile(socketfd, filefd, NULL, MAX_BUFFER -1);
  } while(size_content > 0);

  if (size_content < 0)
  {
    printf("Failed to send file");
    close(filefd);
    return false;
  }

  close(filefd);
  return true;
}
