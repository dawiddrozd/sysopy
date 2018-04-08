CC=gcc
CFLAGS=-g -Wall -std=c99
FILENAME=../results/raport2.txt
BLOCK_SIZE=10000
NUM_OF_ELEMENTS=20000
TEST_NUMBER=10000

.PHONY: clean

all: client_static client_shared client_dynamic

client_static: client.c tests.c ../library/libblocks_static.a
	$(CC) $(CFLAGS) $^ -o $@.out

client_dynamic: client.c tests.c
	$(CC) $(CFLAGS) $^ -o $@.out -L. -lblocks_dynamic

client_shared: client.c tests.c
	$(CC) $(CFLAGS) $^ -o $@.out -ldl -D DYNAMIC

raport2: FILENAME=../results/raport2.txt
raport2: all
	./client_static.out -e $(NUM_OF_ELEMENTS) -o $(BLOCK_SIZE) --static \
    	-a $(TEST_NUMBER) -b $(TEST_NUMBER) -c $(TEST_NUMBER) >> $(FILENAME)

raport3a: FILENAME=../results/raport3a.txt
raport3a: all
	FILENAME=../results/raport3a.txt
	echo "STATIC LINKED\n" > $(FILENAME)
	./client_static.out -e $(NUM_OF_ELEMENTS) -o $(BLOCK_SIZE) --static \
	-a $(TEST_NUMBER) -b $(TEST_NUMBER) -c $(TEST_NUMBER) >> $(FILENAME)

	echo "\n\nDYNAMIC LINKED\n" >> $(FILENAME)
	./client_dynamic.out -e $(NUM_OF_ELEMENTS) -o $(BLOCK_SIZE) --dynamic \
	-a $(TEST_NUMBER) -b $(TEST_NUMBER) -c $(TEST_NUMBER) >> $(FILENAME)

	echo "\n\nSHARED LINKED\n" >> $(FILENAME)
	./client_shared.out -e $(NUM_OF_ELEMENTS) -o $(BLOCK_SIZE) --dynamic \
	-a $(TEST_NUMBER) -b $(TEST_NUMBER) -c $(TEST_NUMBER) >> $(FILENAME)

raport3b: FILENAME=../results/raport3b.txt
raport3b: all
	./client_static.out -e $(NUM_OF_ELEMENTS) -o $(BLOCK_SIZE) --dynamic \
		-a $(TEST_NUMBER) >> $(FILENAME)

o0: CFLAGS+= -O0
o0: raport3b

o1: CFLAGS+= -O1
o1: raport3b

o2: CFLAGS+= -O2
o2: raport3b

o3: CFLAGS+= -O3
o3: raport3b

oS: CFLAGS+= -Os
oS: raport3b

do: FILENAME=../results/raport3b.txt
do: all
	echo "\nOptymalizacja O0" > $(FILENAME); make o0
	echo "\nOptymalizacja O1" >> $(FILENAME); make o1
	echo "\nOptymalizacja O2" >> $(FILENAME); make o2
	echo "\nOptymalizacja O3" >> $(FILENAME); make o3
	echo "\nOptymalizacja Os" >> $(FILENAME); make oS

clean:
	rm -f *.out *.o