
CC = clang
CFLAGS = -Wall -g
LDFLAGS = -lm -lSDL2
SRC = src
BIN = bin
SOURCES = $(wildcard $(SRC)/*.c)
HEADERS = $(wildcard $(SRC)/*.h)
EXE = $(BIN)/life
EXE_PROF = $(EXE)_prof

all : $(BIN) $(EXE)

$(BIN) :
	mkdir -p $(BIN)

$(EXE) : $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(EXE)

$(EXE_PROF) : $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -pg $(LDFLAGS) $(SOURCES) -o $(EXE_PROF)

clean :
	rm -rf $(BIN)

profiled : $(EXE_PROF)

analyze_profile :
	gprof $(EXE_PROF) gmon.out > prof.dat

.PHONY : all clean profiled analyze_profile
