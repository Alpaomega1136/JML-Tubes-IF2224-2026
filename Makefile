CC     = g++
CFLAGS = -Wall -std=c++17 -I src
TARGET = lexer
SRCS   = src/main.cpp src/lexer/lexer.cpp src/lexer/token.cpp src/parser/parser.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
