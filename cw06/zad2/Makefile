CC=gcc
CFLAGS=-Wall -lrt -std=c99

.PHONY: all

all: client/main.c common/utils.c server/main.c
	$(CC) $(CFLAGS) -o client.out client/main.c common/utils.c
	$(CC) $(CFLAGS) -o server.out server/main.c common/utils.c

clean:
	rm -f *.out *.txt