#ifdef DYNAMIC
#include <dlfcn.h>
#endif

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
    char *allocation_method;
    char *test;
    int test_number;

} Args;

Args process_arguments(int argc, char **argsv);

void print_args(Args args);

void load_data(Block *block, int num_elements);

Block* test_a(Args args);

Block* test_b(Args args);

Block* test_c(Args args);

//liczba elementów, rozmiar bloku, sposób alokacji, spis wykonywanych operacji
int main(int argc, char **argsv) {
//
#ifdef DYNAMIC
    lib = dlopen("../library/libblocks_dynamic.so", RTLD_LAZY);
    //void (*print)(Block*) = dlsym(lib, "print");
    void (*delete_all)(Block*) = dlsym(lib, "delete_all");
#endif

    Args args = process_arguments(argc, argsv);
    //print_args(args);
    Block *block;

    switch(args.test[1]) {
        case 'a':
            block = test_a(args);
            break;
        case 'b':
            block = test_b(args);
            break;
        case 'c':
            block = test_c(args);
            break;
        default:
            printf("Can't parse fourth argument: Expected -a -b -c\n");
            exit(EXIT_FAILURE);
    }

    delete_all(block);

#ifdef DYNAMIC
    dlclose(lib);
#endif
    return 0;
}

Args process_arguments(int argc, char **argsv) {
    if (argc != 6) {
        printf("Incorrect number of arguments: 5 expected.\n");
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

    char *allocation_method = argsv[3];
    if (strcmp(allocation_method, "-static") != 0
        && strcmp(allocation_method, "-dynamic") != 0) {
        printf("Can't parse third argument: Expected -static or -dynamic\n");
        exit(EXIT_FAILURE);
    }

    char *fourth = argsv[4];
    if(fourth[0] != (char) '-' && strlen(fourth)!=2) {
        printf("Can't parse fourth argument: Expected -a -b -c\n");
        exit(EXIT_FAILURE);
    }

    int fifth = (int) strtol(argsv[5], &errptr, 10);
    if (*errptr != '\0') {
        printf("Can't parse fifth argument: Only numbers are expected.\n");
        exit(EXIT_FAILURE);
    }

    Args args;
    args.num_elements = num_elements;
    args.block_size = block_size;
    args.allocation_method = allocation_method;
    args.test = fourth;
    args.test_number = fifth;

    if(args.test_number > args.num_elements) {
        printf("Number of elements to delete should be smaller than used elements.\n");
        exit(EXIT_FAILURE);
    }

    return args;
}

void print_args(Args args) {
    printf("num_elements = %d\n"
                   "block_size = %d\n",
           args.num_elements, args.block_size);

}

// stworzenie tablicy,
// wyszukanie elementu
// usunięcie i dodanie zadanej liczby bloków
/* -a 2000 */
Block* test_a(Args args) {
    Block* block = test_create_block(args.num_elements, args.block_size);
    test_search(block);
    test_del_add(block, args.test_number);
    return block;
}

//stworzenie tablicy, usunięcie i dodanie zadanej liczby bloków
Block* test_b(Args args) {
    Block* block = test_create_block(args.num_elements, args.block_size);
    test_del_add(block, args.test_number);
    //print(block);
    return block;
}

//stworzenie tablicy, naprzemienne usunięcie i dodanie zadanej liczby bloków).
Block* test_c(Args args) {
    Block* block = test_create_block(args.num_elements, args.block_size);
    test_del_add_alternally(block, args.test_number);
    return block;
}