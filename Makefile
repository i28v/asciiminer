CC=gcc
CFLAGS=-Wall -Os -std=c99 -pedantic
LDFLAGS=-s -Os
INCLUDES=
LIBS=-lncurses
SRC=$(wildcard src/*.c)
OBJ=$(SRC:%.c=%.o)
OUT=miner

$(OUT): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OUT) $(OBJ)

