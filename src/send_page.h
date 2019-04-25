#ifndef SEND_PAGE_H
#define SEND_PAGE_H

typedef enum
{
  INTRO,
  START,
  FIRST_TURN,
  ACCEPTED,
  DISCARDED,
  ENDGAME,
  GAMEOVER
} PAGE_TYPE;

bool send_page(PAGE_TYPE page_type, int socketfd);

#endif
