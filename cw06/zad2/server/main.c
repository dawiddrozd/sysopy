#define _GNU_SOURCE
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include "../common/utils.h"
#include <signal.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <errno.h>

#define MAX_CLIENTS 100
#define SIGINT 2

int CLIENTS_QUEUE[MAX_CLIENTS];
int PUBLIC_QUEUE = -1;

void stop_handler(int signum) {
    printf("[S] SERVER STOPPED!\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (CLIENTS_QUEUE[i] != -1) {
            mq_close(CLIENTS_QUEUE[i]);
            printf("[S] Queue[%d] stopped\n", CLIENTS_QUEUE[i]);
        }
    }
    mq_close(PUBLIC_QUEUE);
    mq_unlink(MAIN_PATH);
    printf("[S] Queue[%d] stopped\n", PUBLIC_QUEUE);
    printf("DONE.\n");
    exit(0);
}

void handle_math(struct msg_buf msgbuf);

void reverse(char s[MAX_MSG_SIZE]) {
    size_t length = strlen(s);
    char c;

    for (size_t i = 0, j = length - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void handle_new(struct msg_buf received_msg) {
    struct msg_buf msg_to_send;
    printf("[S] [INIT] New client: %d\n", received_msg.client_id);

    int client_id = 0;
    while (client_id < MAX_CLIENTS && CLIENTS_QUEUE[client_id] != -1) client_id++;
    if (client_id >= MAX_CLIENTS) {
        printf(RED "[S] [ERROR] Max size!" RESET);
        return;
    }

    CLIENTS_QUEUE[client_id] = mq_open(received_msg.text,O_WRONLY);
    if(CLIENTS_QUEUE[client_id] < 0 ) {
        printf(RED "[C] Can't open queue file with path: %s!: %s\n" RESET, received_msg.text, strerror(errno));
    }

    msg_to_send.mtype = SERVER_INIT;
    msg_to_send.client_id = client_id;

    int snd;
    snd = mq_send(CLIENTS_QUEUE[client_id], (char *) &msg_to_send, MAX_MSG_SIZE, PRIORITY);
    if (snd < 0)
        printf(RED "[S] [ERROR] Problem with sending message: %s\n" RESET, strerror(errno));
    else
        printf("[S] [INIT] Message sent to client[%d]\n", client_id);
}

void handle_mirror(struct msg_buf *msg) {
    printf("[S] [MIRROR] New message received: %d\n", msg->client_id);

    if (CLIENTS_QUEUE[msg->client_id] == -1) {
        printf("[S] [ERROR] Client not registered\n");
        return;
    }
    int client_queue_id = CLIENTS_QUEUE[msg->client_id];
    reverse(msg->text);

    int snd;
    snd = mq_send(client_queue_id, (char *) msg, MAX_MSG_SIZE, PRIORITY);
    if (snd < 0)
        printf("[S] [ERROR] Problem with sending message\n");
    else
        printf("[S] [MIRROR] Message sent to client[%d]\n", msg->client_id);
}

void handle_time(struct msg_buf msg) {
    printf("[S] [TIME] New message received from %d\n", msg.client_id);

    if (CLIENTS_QUEUE[msg.client_id] == -1) {
        printf("[S] [ERROR] Client not registered\n");
        return;
    }
    int client_queue_id = CLIENTS_QUEUE[msg.client_id];
    msg.mtype = SERVER_TIME;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(msg.text, sizeof(msg.text), "%c", tm);

    int snd;
    snd = mq_send(client_queue_id, (char *) &msg, MAX_MSG_SIZE, PRIORITY);
    if (snd < 0)
        printf("[S] [ERROR] Problem with sending message\n");
    else
        printf("[S] [TIME] Message sent to client[%d]\n", msg.client_id);
}

void handle_stop(struct msg_buf msg) {
    printf("[S] [STOP] New message received from %d\n", msg.client_id);
    if (CLIENTS_QUEUE[msg.client_id] == -1) {
        printf("[S] [ERROR] Client not registered\n");
        return;
    }
    int private_queue_id = CLIENTS_QUEUE[msg.client_id];
    mq_close(private_queue_id);
    CLIENTS_QUEUE[msg.client_id] = -1;
}

void reveive() {
    struct msg_buf received_msg;
    bool server_end = false;
    __syscall_slong_t how_many = 1;
    while (how_many > 0) {
        ssize_t size;
        size = mq_receive(PUBLIC_QUEUE, (char*) &received_msg, MAX_MSG_SIZE, NULL);
        received_msg = received_msg;
        IF(size < 0, "[S] [ERROR] Receiving failed\n");

        switch (received_msg.mtype) {
            case SERVER_INIT:
                handle_new(received_msg);
                break;
            case SERVER_MIRROR:
                handle_mirror(&received_msg);
                break;
            case SERVER_TIME:
                handle_time(received_msg);
                break;
            case SERVER_SUB:
            case SERVER_MUL:
            case SERVER_DIV:
            case SERVER_ADD:
                handle_math(received_msg);
                break;
            case SERVER_STOP:
                handle_stop(received_msg);
                break;
            case SERVER_END:
                printf("[S] [END] SERVER END command received\n");
                struct mq_attr attr;
                mq_getattr(PUBLIC_QUEUE, &attr);
                how_many = attr.mq_curmsgs + 1;
                server_end = true;
                break;
            default:
                printf("[S] [ERROR] Unknown message\n");
        }
        if (server_end) how_many--;
    }
    stop_handler(0);
}

void handle_math(struct msg_buf msgbuf) {
    printf("[S] [MATH] New message received from %d\n", msgbuf.client_id);
    int a = msgbuf.nums[0];
    int b = msgbuf.nums[1];
    int result = 0;

    if (CLIENTS_QUEUE[msgbuf.client_id] == -1) {
        printf("[S] [ERROR] Client not registered\n");
        return;
    }
    int client_queue_id = CLIENTS_QUEUE[msgbuf.client_id];

    switch (msgbuf.mtype) {
        case SERVER_SUB:
            result = a - b;
            break;
        case SERVER_MUL:
            result = a * b;
            break;
        case SERVER_DIV:
            result = a / b;
            break;
        case SERVER_ADD:
            result = a + b;
            break;
        default:
            printf("[S] [ERROR] Unknown message.");
            return;
    }

    msgbuf.nums[0] = result;
    int snd;
    snd = mq_send(client_queue_id, (char *) &msgbuf, MAX_MSG_SIZE, PRIORITY);
    if (snd < 0)
        printf("[S] [ERROR] Problem with sending message\n");
    else
        printf("[S] [MATH] Message sent to client[%d]\n", msgbuf.client_id);
}

int main(int argc, char **argv) {
    //create public queue
    struct mq_attr attr;
    attr.mq_flags = O_NONBLOCK;
    attr.mq_maxmsg = MAX_NR_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    PUBLIC_QUEUE = mq_open(MAIN_PATH, O_CREAT | O_RDONLY, 0664, &attr);
    IF(PUBLIC_QUEUE < 0, "[C] Problem with creating public queue");

    printf("[S] Common queue opened with ID[%d]\n", PUBLIC_QUEUE);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        CLIENTS_QUEUE[i] = -1;
    }

    signal(SIGINT, stop_handler);
    reveive();

    return 0;
}