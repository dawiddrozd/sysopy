#define _GNU_SOURCE

#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/sem.h>
#include <time.h>
#include <semaphore.h>
#include "common.h"

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

void sem_give(sem_t *semaphore) {
    if(sem_post(semaphore) == -1) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), "Incrementing semaphore failed.");
    }
}

void sem_take(sem_t *semaphore) {
    if (sem_wait(semaphore) < 0) {
        printf("%s", strerror(errno));
    }
}

char *gettime(char buffer[DATE_SIZE]) {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    sprintf(buffer, "%d:%li",(int)time.tv_sec, time.tv_nsec);
    return buffer;
}