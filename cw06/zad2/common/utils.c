//
// Created by Dawid Drozd on 23.04.2018.
//

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

const char *homedir() {
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

int get_common_key() {
    key_t common_key;
    common_key = ftok(homedir(), COMMON_KEY);
    IF(common_key < 0, "[C] Can't generate common key");

    return common_key;
}

int get_private_key() {
    key_t private_key;
    private_key = ftok(homedir(), COMMON_KEY) + getpid();
    IF(private_key < 0, "[C] Can't generate private key");

    return private_key;
}