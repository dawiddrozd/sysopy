CC=gcc
CFLAGS=-Wall -std=c99

.PHONY: server client

client: client/main.c common/utils.c
	$(CC) $(CFLAGS) -o client.out $^

server: server/main.c common/utils.c
	$(CC) $(CFLAGS) -o server.out $^

clean:
	rm -f *.out *.txt