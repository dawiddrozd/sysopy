#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <stdbool.h>
#include <signal.h>
#include <mqueue.h>
#include "../common/utils.h"

#define SIGINT 2

int MY_ID = -1;
mqd_t PUBLIC_QUEUE_ID = -1;
mqd_t PRIVATE_QUEUE_ID = -1;
char MY_PATH[MAX_TEXT];

void stop_handler(int signum) {
    printf("CLIENT STOPPED!\n");
    struct msg_buf send_msg;
    send_msg.client_id = MY_ID;
    printf(GREEN "[C] SERVER END command\n" RESET);
    int snd = mq_send(PUBLIC_QUEUE_ID, (char *) &send_msg, MAX_MSG_SIZE, PRIORITY);
    IF(snd < 0, "[C] Problem with sending END message");
    // clean
    mq_close(PRIVATE_QUEUE_ID);
    unlink(MY_PATH);
    exit(0);
}

int to_command(char *cmd) {
    if (strcmp(cmd, "ADD") == 0) return SERVER_ADD;
    if (strcmp(cmd, "MUL") == 0) return SERVER_MUL;
    if (strcmp(cmd, "SUB") == 0) return SERVER_SUB;
    if (strcmp(cmd, "DIV") == 0) return SERVER_DIV;
    if (strcmp(cmd, "INIT") == 0) return SERVER_INIT;
    if (strcmp(cmd, "MIRROR") == 0) return SERVER_MIRROR;
    if (strcmp(cmd, "TIME") == 0) return SERVER_TIME;
    if (strcmp(cmd, "STOP") == 0) return SERVER_STOP;
    if (strcmp(cmd, "END") == 0) return SERVER_END;
    return -1;
}

int to_int(char *string) {

    char *errptr;
    int number = (int) strtol(string, &errptr, 10);
    if (*errptr != '\0') {
        fprintf(stderr, "[C] Can't parse number.\n");
        return -1;
    }
    return number;
}

void send_and_receive(struct msg_buf *send_msg,
                      struct msg_buf *received_msg) {
    int snd;
    snd = mq_send(PUBLIC_QUEUE_ID, (char *) send_msg, MAX_MSG_SIZE, PRIORITY);
    IF(snd < 0, "[C] Problem with sending message");
    ssize_t size;
    size = mq_receive(PRIVATE_QUEUE_ID, (char *) received_msg, MAX_MSG_SIZE, NULL);
    IF(size < 0, "[S] Receiving failed");
}

void run(FILE *file) {
    struct msg_buf send_msg;
    struct msg_buf received_msg;
    char line[MAX_CMDS];

    bool running = true;
    while (running && fgets(line, MAX_MSG_SIZE, file)) {
        char *p = strtok(line, " \t\n");
        send_msg.mtype = to_command(p);
        switch (send_msg.mtype) {
            case SERVER_INIT:
                printf(GREEN "[C] SERVER_INIT init command sending with ID[%d]\n" RESET, PRIVATE_QUEUE_ID);
                send_msg.client_id = getpid();
                sprintf(send_msg.text, "%s", MY_PATH);
                send_and_receive(&send_msg, &received_msg);
                MY_ID = received_msg.client_id;
                printf(BLUE "[C] SERVER_INIT message received from server ID[%d]\n" RESET, received_msg.client_id);
                break;
            case SERVER_MIRROR:
                printf(GREEN "[C] SERVER_MIRROR command sending: %s\n" RESET, p);
                p = strtok(NULL, "\n");
                send_msg.client_id = MY_ID;
                sprintf(send_msg.text, "%s", p);
                send_and_receive(&send_msg, &received_msg);
                printf(BLUE "[C] SERVER_MIRROR message received: %s\n" RESET, received_msg.text);
                break;
            case SERVER_TIME:
                printf(GREEN "[C] SERVER_TIME command sending: %s\n" RESET, p);
                send_msg.client_id = MY_ID;
                send_and_receive(&send_msg, &received_msg);
                printf(BLUE "[C] SERVER_TIME message received: %s\n" RESET, received_msg.text);
                break;
            case SERVER_ADD:
            case SERVER_DIV:
            case SERVER_MUL:
            case SERVER_SUB:
                printf(GREEN "[C] SERVER MATH command sending: %s\n" RESET, p);
                send_msg.client_id = MY_ID;
                int a = to_int(strtok(NULL, " \t\n"));
                int b = to_int(strtok(NULL, " \t\n"));
                send_msg.nums[0] = a;
                send_msg.nums[1] = b;
                if (a >= 0 && b >= 0) {
                    send_and_receive(&send_msg, &received_msg);
                }
                printf(BLUE "[C] SERVER MATH message received: %d\n" RESET, received_msg.nums[0]);
                break;
            case SERVER_STOP:
            case SERVER_END:
                send_msg.client_id = MY_ID;
                printf(GREEN "[C] SERVER STOP/END command\n" RESET);
                int snd = mq_send(PUBLIC_QUEUE_ID, (char *) &send_msg, MAX_MSG_SIZE, PRIORITY);
                IF(snd < 0, "[C] Problem with sending mirror message");
                running = false;
                break;
            default:
                printf("[ERROR] Unknown command \n");
        }
    }
}

int main(int argc, char **argv) {
    IF(argc < 2, "One parameter expected! Example: ./client.out test.txt");

    FILE *file = fopen(argv[1], "r");
    IF(file == NULL, "Can't find file");

    //open public
    PUBLIC_QUEUE_ID = mq_open(MAIN_PATH, O_WRONLY);
    IF(PUBLIC_QUEUE_ID < 0, "Can't open public queue");

    // open private
    char path[MAX_TEXT];
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 16;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    sprintf(path, "%s%d", MAIN_PATH, getpid());
    sprintf(MY_PATH, "%s", path);
    PRIVATE_QUEUE_ID = mq_open(path, O_CREAT | O_RDWR, 0664, &attr);
    IF(PRIVATE_QUEUE_ID < 0, "Can't open private queue");
    printf("[C] My path: %s\n", path);

    signal(SIGINT, stop_handler);
    run(file);

    mq_close(PRIVATE_QUEUE_ID);
    unlink(MY_PATH);

    return 0;
}