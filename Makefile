CC=g++
SRC=src/*.cpp
BIN=bin/client
FLAGS= -std=c++17 -Wall -Wextra
PACK=xkoprd00

all: $(BIN)
	
$(BIN): $(SRC)
	$(CC) $(SRC) $(FLAGS) -o $(BIN)

pack:
	tar -cvf $(PACK).tar isa.lua src/* Makefile

clean:
	rm $(BIN)
