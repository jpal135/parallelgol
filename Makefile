CC = gcc
CFLAGS = -g -Wall -Wextra -std=c11 -lncurses -pthread

TARGETS = gol

GOL_LIB=gol.o

all: $(TARGETS)

gol: main.c $(GOL_LIB)
	$(CC) $(CFLAGS) -o $@ $^

$(GOL_LIB): gol.c gol.h
		$(CC) -c $(CFLAGS) $<

clean:
	$(RM) $(TARGETS) $(GOL_LIB)

