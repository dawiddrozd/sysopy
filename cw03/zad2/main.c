//
// Created by Dawid Drozd on 25.03.2018.
//

#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <stdbool.h>
#include <errno.h>
#include <memory.h>

const int MAX_NR_OF_ARGS = 64;
const int MAX_LINE_SIZE = 256;

void CHECK(bool is_ok, const char *message) {
    if (!is_ok) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), message);
        exit(errno);
    }
}

int main(int argc, char *argsv[]) {
    if (argc < 2) {
        fprintf(stderr, "Program should have 1 argument! Actual: %d\n", argc - 1);
        fprintf(stderr, "Usage: ./main.out test.txt\n");
        exit(1);
    }
    FILE *file = fopen(argsv[1], "r");
    CHECK(file != NULL, "Can't find file");
    char line[MAX_LINE_SIZE];
    char *parameters[MAX_NR_OF_ARGS];

    while (fgets(line, MAX_LINE_SIZE, file)) {
        int nr_of_args = 0;
        for (char *p = strtok(line, " \n\t"); p != NULL; p = strtok(NULL, " \n\t")) {
            if(nr_of_args >= MAX_NR_OF_ARGS) {
                fprintf(stderr, "Error: %s\n", "You exceed arguments limit!");
                exit(1);
            }
            parameters[nr_of_args] = p;
            nr_of_args++;
        }
        parameters[nr_of_args] = NULL;

        pid_t pid = fork();
        CHECK(pid >= 0, "Fork failed");
        if (pid == 0) {
            CHECK(execvp(parameters[0], parameters) >= 0, "Execvp failed.");
        } else {
            int status;
            wait(&status);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                perror("Something went wrong in child process.");
            }
        }
    }

    return 0;
}