CC=gcc
CFLAGS=-Wall -std=c99
BARBER_SRC=barber.c common.c
CLIENT_SRC=client.c common.c

compile:
	$(CC) $(CFLAGS) -o client.out $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o barber.out $(BARBER_SRC)

clean:
	rm -f *.out *.txt