#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <lzma.h>
#include <string.h>
#include <ntsid.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include "lib.h"

static const int BUFFER = 128;
static const char *DELS = "\n";
#define CLK sysconf(_SC_CLK_TCK)
static struct tms begin;
static clock_t clock_curr;

typedef struct Args {
    int num_elements;
    int block_size;
    char *allocation_type;
    int num_operations;
    char **operations;
} Args;

void print_times(struct tms *beg, clock_t clock_before) {
    struct tms end;
    times(&end);
    printf("Real: %.6f, System: %.6f, User: %.6f\n", (double) (clock() - clock_before) / CLOCKS_PER_SEC,
           (double) (end.tms_stime - beg->tms_stime) / CLK, (double) (end.tms_utime - beg->tms_utime) / CLK);
}

Args process_arguments(int argc, char **argsv);

void printArgs(Args args);

void load_data(Block *block, int num_elements);

Block *test_create_block(int num_elements,int block_size);

char *test_search(Block *block);

void test_del_add(Block *block, int size);

void test_del_add_alternally(Block *block, int size);

//liczba elementów, rozmiar bloku, sposób alokacji, spis wykonywanych operacji
int main(int argc, char **argsv) {
    Args args = process_arguments(argc, argsv);
    //printArgs(args);

    Block *block = test_create_block(args.num_elements,args.block_size);
    //char* found = test_search(block);
    test_search(block);
    test_del_add(block, args.num_elements / 2);
    //delete_all(block);
    test_del_add_alternally(block,args.num_elements/2);

    //print(block);

    return 0;
}

Args process_arguments(int argc, char **argsv) {
    if (argc < 4) {
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

    if (!(strcmp(argsv[3], "-static") == 0 || strcmp(argsv[4], "-dynamic") == 0)) {
        printf("Can't parse third argument: '-static' or '-dynamic' expected.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 4; i <= argc; i++) {
        //printf("%s", argsv[i]);
    }

    Args args;
    args.num_elements = num_elements;
    args.block_size = block_size;
    args.allocation_type = argsv[3];
    args.num_operations = argc - 4;
    args.operations = argsv + 3;

    return args;
}

void printArgs(Args args) {
    printf("num_elements = %d\n"
                   "block_size = %d\n"
                   "allocation_type = %s\n"
                   "num_operations = %d\n",
           args.num_elements, args.block_size, args.allocation_type, args.num_operations);

}

void load_data(Block *block, int num_elements) {
    char *path = "/Users/dawid/CLionProjects/sysopy1/library/data.txt";
    char *line = malloc(BUFFER);
    FILE *file = fopen(path, "r");

    for (int i = 0; i < num_elements; i++) {

        if (fgets(line, BUFFER, file) != NULL) {
            line = strtok(line, DELS);
            add(block, line);
        } else {
            fclose(file);
            file = fopen(path, "r");
        }
    }
    fclose(file);
    free(line);
}

// stworzenie tablicy z zadaną liczbą bloków o zdanym rozmiarze i przy pomocy wybranej funkcji alokującej
// -create 350 500
Block *test_create_block(int num_elements,int block_size) {
    printf("Blocks creation:\n");
    times(&begin);
    clock_curr = clock();
    Block *block = create(block_size);
    load_data(block, num_elements);
    print_times(&begin, clock_curr);

    return block;
}

// wyszukanie najbardziej podobnego elementu z punktu widzenia sumy znaków do elementu zadanego jako argument
// -find
char *test_search(Block *block) {
    printf("Searching:\n");
    times(&begin);
    clock_curr = clock();
    char *found = search_for(block);
    print_times(&begin, clock_curr);

    return found;
}

// usunięcie kolejno zadanej liczby bloków a następnie dodanie  na ich miejsce nowych bloków
// -deladd 100
void test_del_add(Block *block, int size) {
    printf("Deleting and adding in sequence:\n");
    times(&begin);
    clock_curr = clock();
    delete_some(block, size);
    load_data(block, size);
    print_times(&begin, clock_curr);
}

void test_del_add_alternally(Block *block, int size) {
    printf("Deleting and adding alternally:\n");
    times(&begin);
    clock_curr = clock();
    for(int i=0; i<size; i++) {
        delete_some(block,1);
        load_data(block,1);
    }
    print_times(&begin, clock_curr);
}




