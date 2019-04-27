OBJS	= image_tagger.o send_page.o handle_http_request.o play_game.o
HEADER	= src/image_tagger.h src/send_page.h src/handle_http_request.h src/play_game.h
OUT	= image_tagger
CC	 = gcc
FLAGS	 = -c -Wall -Wpedantic #-g

all: mkbin $(OBJS)
	$(CC) -g bin/image_tagger.o bin/send_page.o bin/handle_http_request.o bin/play_game.o -o $(OUT)

image_tagger.o: src/image_tagger.c $(HEADER)
	$(CC) $(FLAGS) src/image_tagger.c -o bin/image_tagger.o -std=c99

handle_http_request.o: src/handle_http_request.c $(HEADER)
	$(CC) $(FLAGS) src/handle_http_request.c -o bin/handle_http_request.o -std=c99

send_page.o: src/send_page.c $(HEADER)
	$(CC) $(FLAGS) src/send_page.c -o bin/send_page.o -std=c99

play_game.o: src/play_game.c $(HEADER)
	$(CC) $(FLAGS) src/play_game.c -o bin/play_game.o -std=c99

mkbin:
	mkdir -p bin

clean:
	rm -f $(OUT) -r bin
