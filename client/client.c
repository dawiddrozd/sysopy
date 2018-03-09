//#ifdef DYNAMIC
//#include <dlfcn.h>
//#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <lzma.h>
#include <string.h>
#include <ntsid.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include "../library/blocks.h"
#include "tests.h"

typedef struct Args {
    int num_elements;
    int block_size;
} Args;

Args process_arguments(int argc, char **argsv);

void printArgs(Args args);

void load_data(Block *block, int num_elements);

//liczba elementów, rozmiar bloku, sposób alokacji, spis wykonywanych operacji
int main(int argc, char **argsv) {
//
//#ifdef DYNAMIC
//    void *lib = dlopen("../library/libblocks_dynamic.so", RTLD_LAZY);
//    Block* (*create)(int, int) = dlsym(lib, "create");
//    void (*add)(Block*, char*) = dlsym(lib, "add");
//    void (*print)(Block*) = dlsym(lib, "print");
//    void (*delete_char)(Block*, const char *) = dlsym(lib, "delete_char");
//    void (*delete_all)(Block*) = dlsym(lib, "delete_all");
//    char* (*search_for)(Block *) = dlsym(lib, "search_for");
//    void (*delete_some)(Block *, int) = dlsym(lib, "delete_some");
//#endif

    Args args = process_arguments(argc, argsv);
    //printArgs(args);


    Block *block = test_create_block(args.num_elements, args.block_size);
    //char* found = test_search(block);
    test_search(block);
    test_del_add(block, args.num_elements / 2);
    //print(block);
    //delete_all(block);
    test_del_add_alternally(block, args.num_elements / 2);
    //print(block);

    //delete_all(block);

    //print(block);
//#ifdef DYNAMIC
//    dlclose(lib);
//#endif
    return 0;
}

Args process_arguments(int argc, char **argsv) {
    if (argc < 2) {
        printf("Incorrect number of arguments: 4 expected.\n");
        exit(EXIT_FAILURE);
    }

    char *errptr;
    int num_elements = (int) strtol(argsv[1], &errptr, 10);
    if (*errptr != '\0') {
        printf("Can't parse first argument: Only numbers are expected.\n");
        exit(EXIT_FAILURE);
    }

    int block_size = (int) strtol(argsv[2], &errptr, 10);
    if (*errptr != '\0') {
        printf("Can't parse second argument: Only numbers are expected.\n");
        exit(EXIT_FAILURE);
    }

    Args args;
    args.num_elements = num_elements;
    args.block_size = block_size;

    return args;
}

void printArgs(Args args) {
    printf("num_elements = %d\n"
                   "block_size = %d\n",
           args.num_elements, args.block_size);

}