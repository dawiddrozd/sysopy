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
#include <getopt.h>
#include "../library/blocks.h"
#include "tests.h"

typedef struct Args {
    int num_elements;
    int block_size;
    int is_dynamic;
    int test_number;
} Args;

int parse_string(char *string);

Block *test_a(Args args);

Block *test_b(Args args);

Block *test_c(Args args);

//liczba elementów, rozmiar bloku, sposób alokacji, spis wykonywanych operacji
int main(int argc, char *argsv[]) {
//
#ifdef DYNAMIC
    lib = dlopen("../library/libblocks_dynamic.so", RTLD_LAZY);
    if(lib == NULL) {
        fprintf(stderr,"Library not found.");
        exit(EXIT_FAILURE);
    }
    //void (*print)(Block*) = dlsym(lib, "print");
    void (*delete_all)(Block*) = dlsym(lib, "delete_all");
#endif

    int opt = 0;
    Args args;
    args.num_elements = 10000;
    args.test_number = 5000;
    args.is_dynamic = 0;
    args.block_size = 128;

    int is_test_a = 0;
    int is_test_b = 0;
    int is_test_c = 0;

    static struct option long_options[] = {
            {"elements",   required_argument, 0, 'e'},
            {"block_size", required_argument, 0, 'o'},
            {"dynamic",    no_argument,       0, 'd'},
            {"static",     no_argument,       0, 's'},
            {"is_test_a",  required_argument, 0, 'a'},
            {"is_test_b",  required_argument, 0, 'b'},
            {"is_test_c",  required_argument, 0, 'c'},
            {0, 0,                            0, 0}
    };

    int long_index = 0;
    while ((opt = getopt_long(argc, argsv, "a:b:c:e:dso:",
                              long_options, &long_index)) != -1) {
        switch (opt) {
            case 'a' :
                is_test_a = parse_string(optarg);
                break;
            case 'b' :
                is_test_b = parse_string(optarg);
                break;
            case 'c' :
                is_test_c = parse_string(optarg);
                break;
            case 'd' :
                args.is_dynamic = 1;
                break;
            case 'e' :
                args.num_elements = parse_string(optarg);
                break;
            case 's' :
                args.is_dynamic = 0;
                break;
            case 'o' :
                args.block_size = parse_string(optarg);
                break;
            default:
                fprintf(stderr, "Incorrect argument.\n");
                exit(EXIT_FAILURE);
        }
    }

    Block *block;

    if (is_test_a) {
        args.test_number = is_test_a;
        block = test_a(args);
        delete_all(block);
    }

    if (is_test_b) {
        args.test_number = is_test_b;
        block = test_b(args);
        delete_all(block);
    }

    if (is_test_c) {
        args.test_number = is_test_c;
        block = test_c(args);
        delete_all(block);
    }

#ifdef DYNAMIC
    dlclose(lib);
#endif
    return 0;
}

int parse_string(char *string) {
    char *errptr;
    int number = (int) strtol(string, &errptr, 10);
    if (*errptr != '\0') {
        printf("Can't parse second argument: Only numbers are expected.\n");
        exit(EXIT_FAILURE);
    }
    return number;
}

// stworzenie tablicy, wyszukanie elementu, usunięcie i dodanie zadanej liczby bloków
Block *test_a(Args args) {
    printf("TEST A:\n");
    Block *block = test_create_block(args.num_elements, args.block_size,!args.is_dynamic);
    test_search(block);
    test_del_add(block, args.test_number);
    return block;
}

//stworzenie tablicy, usunięcie i dodanie zadanej liczby bloków
Block *test_b(Args args) {
    printf("\n\nTEST B:\n");
    Block *block = test_create_block(args.num_elements, args.block_size,!args.is_dynamic);
    test_del_add(block, args.test_number);
    //print(block);
    return block;
}

//stworzenie tablicy, naprzemienne usunięcie i dodanie zadanej liczby bloków).
Block *test_c(Args args) {
    printf("\n\nTEST C:\n");
    Block *block = test_create_block(args.num_elements, args.block_size,!args.is_dynamic);
    test_del_add_alternally(block, args.test_number);
    return block;
}