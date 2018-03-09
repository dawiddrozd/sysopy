CC=gcc
CFLAGS=-g -Wall -std=c99

.PHONY: clean

all:  client_static

client_static: client.c tests.c ../library/libblocks_static.a
	$(CC) $(CFLAGS) $^ -o $@.out

runAll: all
	./client_static.out 1000 128 >> results.txt



