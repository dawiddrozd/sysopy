//
// Created by Dawid Drozd on 23.04.2018.
//

#ifndef SYSOPY4_UTILS_H
#define SYSOPY4_UTILS_H

#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <stdbool.h>
#include <mqueue.h>

#define RED     "\x1b[91m"
#define RESET   "\x1b[0m"
#define BLUE    "\x1b[34m"
#define GREEN   "\x1b[32m"

#define MAX_MSG_SIZE sizeof(struct msg_buf)
#define MAX_TEXT 128
#define MAX_CMDS 16
#define MAX_NR_MESSAGES 64
#define MAIN_PATH "/queue"

#define SERVER_INIT 1
#define SERVER_MIRROR 2
#define SERVER_ADD 3
#define SERVER_MUL 4
#define SERVER_SUB 5
#define SERVER_DIV 6
#define SERVER_STOP 7
#define SERVER_TIME 8
#define SERVER_END 9

#define PRIORITY 1

struct msg_buf {
    long mtype;
    mqd_t client_id;
    char text[MAX_TEXT];
    int nums[2];
};

const char *public_queue_path();

void IF(bool correct, const char *message);

#endif //SYSOPY4_UTILS_H
