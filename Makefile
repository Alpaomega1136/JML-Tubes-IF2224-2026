CC = g++
CFLAGS = -Wall -std=c++17

all: lexer

lexer: src/main.cpp src/lexer/lexer.cpp
    $(CC) $(CFLAGS) -o lexer src/main.cpp src/lexer/lexer.cpp

clean:
    rm -f lexer