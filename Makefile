
CC = clang
CFLAGS = -Wall -g
LDFLAGS = -lm -lSDL2
SRC = src
BIN = bin
SOURCES = $(wildcard $(SRC)/*.c)
HEADERS = $(wildcard $(SRC)/*.h)
EXE = $(BIN)/life

all : $(BIN) $(EXE)

$(BIN) :
	mkdir -p $(BIN)

$(EXE) : $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(EXE)

.PHONY : clean
clean :
	rm -f $(EXE)

