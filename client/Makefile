CC=gcc
CFLAGS=-g -Wall -std=c99
FILENAME=../results/results

.PHONY: clean

all: client_static client_shared client_dynamic results

client_static: client.c tests.c ../library/libblocks_static.a
	$(CC) $(CFLAGS) $^ -o $@.out

client_dynamic: client.c tests.c
	$(CC) $(CFLAGS) $^ -o $@.out -L. -lblocks_dynamic

client_shared: client.c tests.c
	$(CC) $(CFLAGS) $^ -o $@.out -ldl -D DYNAMIC

results:
	echo "STATIC LINKED\n" > $(FILENAME)
	./client_static.out 10000 128 >> $(FILENAME)
	echo "\n\nDYNAMIC LINKED\n" >> $(FILENAME)
	./client_dynamic.out 10000 128 >> $(FILENAME)
	echo "\n\nSHARED LINKED\n" >> $(FILENAME)
	./client_shared.out 10000 128 >> $(FILENAME)

o1: CFLAGS+= -O1
o1: FILENAME=../results/results1.txt
o1: all

o2: CFLAGS+= -O2
o2: FILENAME=../results/results2.txt
o2: all

o3: CFLAGS+= -O3
o3: FILENAME=../results/results3.txt
o3: all

do: all
	make o1
	make o2
	make o3

clean:
	rm -f *.out *.o



