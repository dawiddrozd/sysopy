#define _GNU_SOURCE

#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/sem.h>
#include "common_utils.h"

void check_exit(bool correct, const char *message) {
    if (correct) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), message);
        exit(errno);
    }
}

int to_int(char *string) {
    if (string == NULL) {
        fprintf(stderr, "Can't parse number.\n");
        return -1;
    }

    char *errptr;
    int number = (int) strtol(string, &errptr, 10);
    if (*errptr != '\0') {
        fprintf(stderr, "Can't parse number.\n");
        return -1;
    }
    return number;
}

const char *get_homedir() {
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    return homedir;
}

void inc_sem(int sem_id) {
    struct sembuf sembuf;
    sembuf.sem_flg = SEM_UNDO;
    sembuf.sem_num = 0;
    sembuf.sem_op = 1;

    if (semop(sem_id, &sembuf, 1) < 0) {
        printf("%s", strerror(errno));
    }
}

void dec_sem(int sem_id) {
    struct sembuf sembuf;
    sembuf.sem_flg = SEM_UNDO;
    sembuf.sem_num = 0;
    sembuf.sem_op = -1;

    if (semop(sem_id, &sembuf, 1) < 0) {
        printf("%s", strerror(errno));
    }
}