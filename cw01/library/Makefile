CC=gcc
CFLAGS=-g -Wall -std=c99

.PHONY: clean

all: libblocks_static libblocks_static_obj libblocks_dynamic libblocks_dynamic_obj

#static
libblocks_static_obj: blocks.c
	$(CC) $(CFLAGS) -c $^

# ar -> archiver | $@ - means the target | $^ - means all prerequisites
libblocks_static: blocks.o
	ar crs $@.a $^

#dynamic
libblocks_dynamic_obj: blocks.c
	$(CC) $(CFLAGS) -fPIC -c $^

libblocks_dynamic: blocks.o
	$(CC) -shared $^ -o $@.so

clean:
	rm -f *.a *.o *.so
