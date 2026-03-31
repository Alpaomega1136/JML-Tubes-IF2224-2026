CC     = g++
CFLAGS = -Wall -std=c++17
TARGET = lexer
SRCS   = src/main.cpp src/lexer/lexer.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
