#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <stdio.h>
#include <fcntl.h>
#include <zconf.h>
#include <sys/stat.h>
#include <time.h>

#define BUFF_SIZE 512

void CHECK(bool is_ok, const char *message) {
    if (!is_ok) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), message);
        exit(errno);
    }
}

int parse_string(char *string) {

    char *errptr;
    int number = (int) strtol(string, &errptr, 10);
    if (*errptr != '\0') {
        fprintf(stderr, "Can't parse second argument: Only numbers are expected.\n");
        exit(EXIT_FAILURE);
    }
    return number;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Program should have 2 argument! Actual: %d\n", argc - 1);
        fprintf(stderr, "Usage: ./slave.out 20 10\n");
        exit(1);
    }

    srand((unsigned int) (time(NULL) ^ (getpid())));

    char buffer[BUFF_SIZE];
    char message[BUFF_SIZE*2];

    char *pipepath = argv[1];
    int N = parse_string(argv[2]);

    int descryptor = open(argv[1], O_WRONLY);
    CHECK(descryptor >= 0, "Opening pipe failed!");

    printf("[SLAVE] PID = %d\n", getpid());

    FILE *date = NULL;
    for (int i = 0; i < N; i++) {
        date = popen("date","r");
        CHECK(date != NULL, "Popen failed.");

        int ch;
        int read = 0;
        while ((ch = getc(date)) != EOF){
            buffer[read++] = (char) ch;
        }
        buffer[read] = '\0';

        sprintf(message, "%s from %d", buffer, getpid());
        lseek(descryptor, 0, SEEK_SET);
        write(descryptor, message, strlen(message));

        pclose(date);
        sleep((unsigned int) (rand() % 4 + 2));
    }

    close(descryptor);

    return 0;
}