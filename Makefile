CC=gcc
CFLAGS=-Wall -O2 -std=c99 -pedantic
LDFLAGS=-O2 -s
INCLUDES=
LIBS=
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

