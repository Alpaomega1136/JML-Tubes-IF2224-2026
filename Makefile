CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -g
SRCDIR  = src
BINDIR  = bin

# TODO: tambahkan source file sesuai struktur yang dibuat
# SRCS    = $(SRCDIR)/main.c $(SRCDIR)/lexer.c
# OBJS    = $(SRCS:.c=.o)
# TARGET  = $(BINDIR)/lexer

.PHONY: all clean test

all:
	@echo "TODO: tambahkan target build"

test:
	@echo "TODO: tambahkan test case"

clean:
	rm -f $(SRCDIR)/*.o
	rm -rf $(BINDIR)
