SRC=vjudge.c
OBJECTS=vjudge.o
CC=gcc
CFLAGS=-g -Wall -O4

build: $(SRC)
	$(CC) $(CFLAGS) -o $(OBJECTS) -c $(SRC)

clean:
	rm $(OBJECTS)

.PHONY: build
