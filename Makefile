
CC = clang
CFLAGS = -Wall -g -lm -lSDL2
BIN = ./bin
SOURCES = $(wildcard *.c)
TARGETS = $(addprefix $(BIN)/, $(SOURCES:.c=))

all : $(BIN) $(TARGETS)

$(BIN) :
	mkdir -p $(BIN)

$(BIN)/% : %.c
	$(CC) $(CFLAGS) $< -o $@

