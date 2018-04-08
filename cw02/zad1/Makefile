CC=gcc
CFLAGS=-Wall -std=c99

main: io.c main.c
	$(CC) $(CFLAGS) $^ -o main.out

test: main
	./main.out --test > wyniki.txt

clean:
	rm -f *.out *.txt