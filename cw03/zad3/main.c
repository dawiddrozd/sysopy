//
// Created by Dawid Drozd on 25.03.2018.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <memory.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MILLION 1000000

const int MAX_NR_OF_ARGS = 64;
const int MAX_LINE_SIZE = 256;

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
        printf("Can't parse argument: Only numbers are expected.\n");
        exit(EXIT_FAILURE);
    }
    return number;
}

void display_usage(char *name, struct rusage *before, struct rusage *after) {
    printf(">>> Execution time of %s: \nUser: %ldµs System: %ldµs Data: %ld\n\n", name,
           (after->ru_utime.tv_sec - before->ru_utime.tv_sec) * MILLION + after->ru_utime.tv_usec -
           before->ru_utime.tv_usec,
           (after->ru_stime.tv_sec - before->ru_stime.tv_sec) * MILLION + after->ru_stime.tv_usec -
           before->ru_stime.tv_usec,
           after->ru_idrss - before->ru_idrss);
}

int main(int argc, char *argsv[]) {
    if (argc < 4) {
        fprintf(stderr, "Program should have 3 arguments! Actual: %d\n", argc - 1);
        exit(1);
    }
    int time_limit = parse_string(argsv[2]);
    int memory_limit = parse_string(argsv[3]);

    FILE *file = fopen(argsv[1], "r");
    CHECK(file != NULL, "Can't find file");
    char line[MAX_LINE_SIZE];
    char *parameters[MAX_NR_OF_ARGS];

    //set limits
    struct rlimit rlimit;
    rlimit.rlim_cur = rlimit.rlim_max = (rlim_t) time_limit;
    struct rlimit mem_limit;
    mem_limit.rlim_cur = mem_limit.rlim_max = ((rlim_t) memory_limit * 1024 * 1024);

    while (fgets(line, MAX_LINE_SIZE, file)) {
        struct rusage before, after;
        int nr_of_args = 0;
        for (char *p = strtok(line, " \n\t"); p != NULL; p = strtok(NULL, " \n\t")) {
            parameters[nr_of_args] = p;
            nr_of_args++;
        }
        parameters[nr_of_args] = NULL;

        getrusage(RUSAGE_CHILDREN, &before);

        pid_t pid = fork();
        CHECK(pid >= 0, "Fork failed");
        if (pid == 0) {
            CHECK(setrlimit(RLIMIT_AS, &mem_limit) == 0, "Can't set memory limit");
            CHECK(setrlimit(RLIMIT_CPU, &rlimit) == 0, "Can't set time limit");
            CHECK(execvp(parameters[0], parameters) >= 0, "Execvp failed.");
        } else {
            int status;
            waitpid(pid, &status, 0);
            CHECK((WIFSIGNALED(status) && WEXITSTATUS(status)) == 0, "Child process failed.");

            getrusage(RUSAGE_CHILDREN, &after);
            display_usage(parameters[0], &before, &after);
        }
    }

    return 0;
}
