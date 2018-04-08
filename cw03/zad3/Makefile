CC=gcc
CFLAGS=-Wall -std=c99

all: main loop sleep

main: main.c
	$(CC) $(CFLAGS) $^ -o main.out

loop: loop.c
	$(CC) $(CFLAGS) $^ -o $@.out

sleep: sleep.c
	$(CC) $(CFLAGS) $^ -o $@.out

clean:
	rm -f *.out