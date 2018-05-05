//
// Created by Dawid Drozd on 23.04.2018.
//
#define _GNU_SOURCE
#include <sys/msg.h>
#include <sys/ipc.h>
#include "utils.h"
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <stdbool.h>
#include <memory.h>
#include <stdio.h>

const char *public_queue_path() {
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    return homedir;
}

void IF(bool correct, const char *message) {
    if (correct) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), message);
        exit(errno);
    }
}