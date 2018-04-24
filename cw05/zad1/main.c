#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <memory.h>
#include <unistd.h>

#define RED     "\x1b[91m"
#define RESET   "\x1b[0m"

const int MAX_ARGS = 16;
const int MAX_LINE_SIZE = 256;
const int MAX_PIPES = 16;
const int MAX_ARG_LENGTH = 16;

void CHECK(bool is_ok, const char *message) {
    if (!is_ok) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), message);
        exit(errno);
    }
}

void parse(char *line, char **result) {
    int nr_of_args = 0;

    for (char *p = strtok(line, " \t\n"); p != NULL; p = strtok(NULL, " \t\n")) {
        if (nr_of_args >= MAX_ARGS) {
            fprintf(stderr, "Error: %s\n", "You exceed arguments limit!");
            exit(1);
        }
        strcpy(result[nr_of_args], p);
        nr_of_args++;
    }
    result[nr_of_args] = NULL;
}

void free_array(char **array, int size) {
    for (int i = 0; i < size; i++) {
        free(array[i]);
    }
    free(array);
}

void print_cmd(char *cmds[MAX_PIPES]) {
    printf(RED ">>> " RESET);
    for (int i = 0; cmds[i] != NULL; i++) {
        printf(RED "%s" RESET, cmds[i]);
    }
    printf("\n");
}

char **allocate_array() {
    char **result = malloc(MAX_ARGS * sizeof(char *));
    for (int i = 0; i < MAX_ARGS; i++) {
        result[i] = malloc(MAX_ARG_LENGTH * sizeof(char));
    }
    return result;
}

void run(char *cmds[MAX_PIPES], int nr_of_cmds) {
    int fd[2];
    pid_t pid;
    int fd_in;

    print_cmd(cmds);
    for (int i = 0; i < nr_of_cmds; i++) {
        char **result = allocate_array();
        parse(cmds[i], result);

        pipe(fd);
        pid = fork();

        CHECK(pid >= 0, "Fork failed");
        if (pid == 0) {
            if (i != 0) {
                dup2(fd_in, STDIN_FILENO);
            }
            if (cmds[i + 1] != NULL) {
                dup2(fd[1], STDOUT_FILENO);
            }
            close(fd[0]);
            execvp(result[0], result);
            exit(EXIT_FAILURE);
        } else {
            int status;
            waitpid(pid, &status, 0);
            close(fd[1]);
            fd_in = fd[0];
        }
        free_array(result, MAX_ARGS);
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
    char *cmds[MAX_PIPES];

    int nr_of_cmds = 0;
    while (fgets(line, MAX_LINE_SIZE, file) && nr_of_cmds < MAX_PIPES) {
        nr_of_cmds = 0;
        for (char *p = strtok(line, "|\n"); p != NULL; p = strtok(NULL, "|\n")) {
            cmds[nr_of_cmds] = p;
            nr_of_cmds++;
        }
        cmds[nr_of_cmds] = NULL;
        run(cmds, nr_of_cmds);
    }

    if (nr_of_cmds >= MAX_PIPES) {
        fprintf(stderr, "Error: %s\n", "You exceed arguments limit!");
        exit(1);
    }

    return 0;
}