
CC = clang
CFLAGS = -Wall -g -lm -lSDL2

all : bin bin/life

bin :
	mkdir -p bin

bin/life : src/life.c
	$(CC) $(CFLAGS) src/life.c src/actions.c src/render.c -o bin/life

