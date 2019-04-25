OBJS	= image_tagger.o send_page.o
HEADER	= send_page.h
OUT	= image_tagger
CC	 = gcc
FLAGS	 = -c -Wall -Wpedantic -g

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

image_tagger.o: image_tagger.c $(HEADER)
	$(CC) $(FLAGS) image_tagger.c -std=c99

send_page.o: send_page.c $(HEADER)
	$(CC) $(FLAGS) send_page.c -std=c99

clean:
	rm -f $(OUT) *.o
