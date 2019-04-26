#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "image_tagger.h"
#include "send_page.h"

#define PATH_INTRO "html_files/1_intro.html"
#define PATH_START "html_files/2_start.html"
#define PATH_FIRST_TURN "html_files/3_first_turn.html"
#define PATH_ACCEPTED "html_files/4_accepted.html"
#define PATH_DISCARDED "html_files/5_discarded.html"
#define PATH_ENDGAME "html_files/6_endgame.html"
#define PATH_GAMEOVER "html_files/7_gameover.html"
#define TEMP_FILE "html_files/temp.html"

#define HTTP_200_FORMAT "HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Content-Length: %ld\r\n\r\n"

static void send_html(char * file_path, int socket);

void send_page(PAGE_TYPE page_type, int socket, char * insert_string)
{
  char * path;

  switch(page_type){
    case INTRO: path = PATH_INTRO;
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
    perror("Not a valid page\n");
    exit(EXIT_FAILURE);
  }

  if (insert_string){
    // create a temporary file with insert_string inserted in place of #
    FILE *html = fopen(path, "r");
    FILE *temp = fopen(TEMP_FILE, "w");

    int c = fgetc(html);
    while (c != '#')
    {
      fputc(c, temp);
      c = fgetc(html);
    }

    fputs(insert_string, temp);
    c = fgetc(html);

    while (c != EOF)
    {
      fputc(c, temp);
      c = fgetc(html);
    }

    fclose(html);
    fclose(temp);

    // send the newly created temp file
    send_html(TEMP_FILE, socket);

    // delete the temp file
    remove(TEMP_FILE);
  } else {
    send_html(path, socket);
  }
}

// send the html file at file_path to socket
static void send_html(char * file_path, int socket) {
  char buffer[MAX_BUFFER] = {0};
  int size_content;
  int filefd;
  struct stat file_stats;

  // get stats if html file
  stat(file_path, &file_stats);

  // compose http header
  size_content = sprintf(buffer, HTTP_200_FORMAT, file_stats.st_size);

  // send http header
  if (write(socket, buffer, size_content) < 0)
  {
    perror("Failed to send header\n");
    exit(EXIT_FAILURE);
  }

  filefd = open(file_path, O_RDONLY);
  do
  {
    size_content = read(filefd, buffer, MAX_BUFFER - 1);
    send(socket, buffer, size_content, MSG_NOSIGNAL);
  } while(size_content > 0);

  if (size_content < 0)
  {
    close(filefd);
    perror("Failed to send html file\n");
    exit(EXIT_FAILURE);
  }

  close(filefd);
}
