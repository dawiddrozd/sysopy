#include <stdlib.h>
#include <printf.h>
#include <stdio.h>

//
// Created by Dawid Drozd on 26.03.2018.
//
unsigned int to_int(char *string) {
    char *errptr;
    unsigned int number = (unsigned int) strtol(string, &errptr, 10);
    if (*errptr != '\0') {
        printf("Can't parse argument: Only numbers are expected.\n");
        exit(EXIT_FAILURE);
    }
    return number;
}

int main(int argc, char *argv[]) {

    // ALLOCATE ARGV[1] SECONDS
    int i = 2;

    while (i > 0) {
        i++;
        i--;
    }
    return 0;
}
