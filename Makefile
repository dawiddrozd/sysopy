CC=gcc
CFLAGS=-g -Wall -std=c99

.PHONY: clean

all: lib_static lib_static_obj

#static
lib_static: library/client.o library/lib.o
	$(CC) $(CFLAGS) -o client $^

lib_static_obj: library/client.c library/lib.c
	$(CC) $(CFLAGS) -fPIC -c $^

#dynamic
lib_dynamic: main.o lib.o
	$(CC) $(CFLAGS) -fPIC -c $^

lib_dynamic_obj: library/client.c library/lib.c
	$(CC) -shared $^ -o $@.so

clean:
	rm -f *.a *.o *.so main
