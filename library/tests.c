#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <lzma.h>
#include <string.h>
#include <ntsid.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <sys/times.h>
#include "../library/lib.h"
#include "tests.h"

#define CLK sysconf(_SC_CLK_TCK)

static const int BUFFER = 128;
static const char *DELS = "\n";

static struct tms begin;
static clock_t clock_curr;

void print_times(struct tms *beg, clock_t clock_before) {
    struct tms end;
    times(&end);
    printf("Real: %.6f, System: %.6f, User: %.6f\n", (double) (clock() - clock_before) / CLOCKS_PER_SEC,
           (double) (end.tms_stime - beg->tms_stime) / CLK, (double) (end.tms_utime - beg->tms_utime) / CLK);
}

void load_data(Block *block, int num_elements) {
    char *path = "/Users/dawid/CLionProjects/sysopy1/library/data.txt";
    char *line = malloc(BUFFER);
    FILE *file = fopen(path, "r");

    for (int i = 0; i < num_elements; i++) {

        if (fgets(line, BUFFER, file) == NULL) {
            fclose(file);
            file = fopen(path, "r");
        }
        line = strtok(line, DELS);
        add(block, line);
    }
    fclose(file);
    free(line);
}

// stworzenie tablicy z zadaną liczbą bloków o zdanym rozmiarze i przy pomocy wybranej funkcji alokującej
// -create 350 500
Block *test_create_block(int num_elements, int block_size) {
    printf("Blocks creation:\n");
    times(&begin);
    clock_curr = clock();
    Block *block = create(num_elements,block_size);
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

// na przemian usunięcie i dodanie zadanej liczby bloków
void test_del_add_alternally(Block *block, int size) {
    printf("Deleting and adding alternally:\n");
    times(&begin);
    clock_curr = clock();
    for (int i = 0; i < size; i++) {
        delete_some(block, 1);
        load_data(block, 1);
    }
    print_times(&begin, clock_curr);
}