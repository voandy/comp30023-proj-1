#ifndef HANDLE_HTTP_REQUEST_H
#define HANDLE_HTTP_REQUEST_H

#define MAX_STR_SIZE 512

typedef enum
{
    GET,
    POST,
    UNKNOWN
} METHOD;

struct http_req_header
{
  METHOD method;
  char url[MAX_STR_SIZE];
  char protocol[MAX_STR_SIZE];

  bool start;
  bool quit;
  bool guess;

  char user[MAX_STR_SIZE];
  char keyword[MAX_STR_SIZE];
};

bool handle_http_request(int socket, server_state * state,
  player * p1, player * p2, int * player_count);
struct http_req_header parse_req_header(char * req_header_raw);
METHOD parse_method(char * method_raw);

#endif
