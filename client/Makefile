CC=gcc
CFLAGS=-g -Wall -std=c99
OUT_NAME=../results/results.txt

.PHONY: clean

all: client_static client_shared

client_static: client.c tests.c ../library/libblocks_static.a
	$(CC) $(CFLAGS) $^ -o $@.out

client_dynamic: client.c tests.c
	$(CC) $(CFLAGS) $^ -o $@.out -L../library -lblocks_dynamic -Wl,-rpath=../library

client_shared: client.c tests.c
	$(CC) $(CFLAGS) $^ -o $@.out -ldl -D DYNAMIC

results: all
	echo "STATIC LINKED\n" > results.txt
	./client_static.out 1000 128 >> results.txt


clean:
	rm -f *.out



