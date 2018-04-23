#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <stdio.h>
#include <fcntl.h>
#include <zconf.h>
#include <sys/stat.h>

#define BUFF_SIZE 512

void CHECK(bool is_ok, const char *message) {
    if (!is_ok) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), message);
        exit(errno);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Program should have 1 argument! Actual: %d\n", argc - 1);
        fprintf(stderr, "Usage: ./main.out test.txt\n");
        exit(1);
    }

    char buffer[BUFF_SIZE];
    char *pipepath = argv[1];

    //create pipe
    CHECK((mkfifo(pipepath, S_IRWXU | S_IRWXG | S_IRWXO | S_IFIFO) >= 0), "Mkfifo operation failed.");

    int ptr = open(argv[1], O_RDONLY);
    CHECK(ptr >=0, "Opening pipe failed!");

    while (read(ptr, buffer, BUFF_SIZE)) {
        printf("[MASTER] Received: %s\n", buffer);
    }

    close(ptr);

    return 0;
}