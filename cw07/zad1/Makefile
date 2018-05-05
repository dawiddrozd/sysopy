CC=gcc
CFLAGS=-Wall -std=c99
BARBER_SRC=barber/barber.c barber/barber_utils.c common/common_utils.c
CLIENT_SRC=client/client.c client/client_utils.c common/common_utils.c

compile:
	$(CC) $(CFLAGS) -o client.out $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o barber.out $(BARBER_SRC)

clean:
	rm -f *.out *.txt