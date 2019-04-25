#ifndef HANDLE_HTTP_REQUEST_H
#define HANDLE_HTTP_REQUEST_H

#define MAX_URL_SIZE 256

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

bool handle_http_request(int socket, SERVER_STATE * state);
struct http_req_header parse_req_header(char * req_header_raw);
METHOD parse_method(char * method_raw);

#endif
