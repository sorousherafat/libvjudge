SRC=vjudge.c
OBJECTS=vjudge.o
CC=gcc

build: $(SRC)
	$(CC) -o $(OBJECTS) -c $(SRC)

clean:
	rm $(OBJECTS)

.PHONY: build
