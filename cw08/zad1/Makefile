CC=gcc
CFLAGS=-Wall -lpthread -std=c99
MAIN_SRC=main.c
IN_FILE=./in.pgm
FILTER=filter.txt
OUT_FILE=out.pgm
OUT=./main.out
RESULTS=Times.txt

compile:
	$(CC) $(CFLAGS) -o $(OUT) $(MAIN_SRC)

test: compile
	$(OUT) 1 $(IN_FILE) $(FILTER) $(OUT_FILE) > $(RESULTS)
	$(OUT) 2 $(IN_FILE) $(FILTER) $(OUT_FILE) >> $(RESULTS)
	$(OUT) 4 $(IN_FILE) $(FILTER) $(OUT_FILE) >> $(RESULTS)
	$(OUT) 8 $(IN_FILE) $(FILTER) $(OUT_FILE) >> $(RESULTS)

clean:
	rm -f *.out