//
// Created by Dawid Drozd on 23.04.2018.
//

#ifndef SYSOPY4_UTILS_H
#define SYSOPY4_UTILS_H

#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <stdbool.h>

#define RED     "\x1b[91m"
#define RESET   "\x1b[0m"
#define BLUE    "\x1b[34m"
#define GREEN   "\x1b[32m"

#define MAX_BUFF_SIZE 256
#define MAX_CMDS 16

#define SERVER_INIT 1
#define SERVER_MIRROR 2
#define SERVER_ADD 3
#define SERVER_MUL 4
#define SERVER_SUB 5
#define SERVER_DIV 6
#define SERVER_STOP 7
#define SERVER_TIME 8
#define SERVER_END 9

#define COMMON_KEY 's'
#define MSG_BUFF_SIZE (int) sizeof(struct msgbuf) - sizeof(long)

struct msgbuf {
    long mtype;
    int client_id;
    char text[MAX_BUFF_SIZE];
    int nums[2];
};

const char *homedir();

void IF(bool correct, const char *message);

int get_common_key();

int get_private_key();

#endif //SYSOPY4_UTILS_H
