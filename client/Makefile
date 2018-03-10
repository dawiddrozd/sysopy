CC=gcc
CFLAGS=-g -Wall -std=c99
FILENAME=../results/results
BLOCK_SIZE=128
NUM_OF_ELEMENTS=20000
TEST_NUMBER=10000

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
	echo "\nTEST A:" >> $(FILENAME)
	./client_static.out $(NUM_OF_ELEMENTS) $(BLOCK_SIZE) -static -a $(TEST_NUMBER) >> $(FILENAME)
	echo "\nTEST B:" >> $(FILENAME)
	./client_static.out $(NUM_OF_ELEMENTS) $(BLOCK_SIZE) -static -b $(TEST_NUMBER) >> $(FILENAME)
	echo "\nTEST B:" >> $(FILENAME)
	./client_static.out $(NUM_OF_ELEMENTS) $(BLOCK_SIZE) -static -c $(TEST_NUMBER) >> $(FILENAME)

	echo "\n\nDYNAMIC LINKED\n" >> $(FILENAME)
	echo "\nTEST A:" >> $(FILENAME)
	./client_dynamic.out $(NUM_OF_ELEMENTS) $(BLOCK_SIZE) -static -a $(TEST_NUMBER) >> $(FILENAME)
	echo "\nTEST B:" >> $(FILENAME)
	./client_dynamic.out $(NUM_OF_ELEMENTS) $(BLOCK_SIZE) -static -b $(TEST_NUMBER) >> $(FILENAME)
	echo "\nTEST C:" >> $(FILENAME)
	./client_dynamic.out $(NUM_OF_ELEMENTS) $(BLOCK_SIZE) -static -c $(TEST_NUMBER) >> $(FILENAME)

	echo "\n\nSHARED LINKED\n" >> $(FILENAME)
	echo "\nTEST A:" >> $(FILENAME)
	./client_shared.out $(NUM_OF_ELEMENTS) $(BLOCK_SIZE) -static -a $(TEST_NUMBER) >> $(FILENAME)
	echo "\nTEST B:" >> $(FILENAME)
	./client_shared.out $(NUM_OF_ELEMENTS) $(BLOCK_SIZE) -static -b $(TEST_NUMBER) >> $(FILENAME)
	echo "\nTEST C:" >> $(FILENAME)
	./client_shared.out $(NUM_OF_ELEMENTS) $(BLOCK_SIZE) -static -c $(TEST_NUMBER) >> $(FILENAME)


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



