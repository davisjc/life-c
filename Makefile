
CC = clang
SOURCES = src/$(wildcard *.c)
HEADERS = src/$(wildcard *.h)
CFLAGS = -Wall -g -lm -lSDL2

all : bin bin/life

bin :
	mkdir -p bin

bin/life : $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) src/life.c src/actions.c src/render.c -o bin/life

