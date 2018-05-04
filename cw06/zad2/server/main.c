#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include "../common/utils.h"

const int MAX_CLIENTS = 100;
int CLIENTS_QUEUE[MAX_CLIENTS];
int COMMON_QUEUE = -1;

void stop_handler(int signum) {
    printf("[S] SERVER STOPPED!\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (CLIENTS_QUEUE[i] != -1) {
            msgctl(CLIENTS_QUEUE[i], IPC_RMID, NULL);
            printf("[S] Queue[%d] stopped\n", CLIENTS_QUEUE[i]);
        }
    }
    msgctl(COMMON_QUEUE, IPC_RMID, NULL);
    printf("[S] Queue[%d] stopped\n", COMMON_QUEUE);
    printf("DONE.\n");
    exit(0);
}

void handle_math(struct msgbuf msgbuf);

void reverse(char s[MAX_BUFF_SIZE]) {
    size_t length = strlen(s);
    char c;

    for (size_t i = 0, j = length - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void handle_new(struct msgbuf received_msg) {
    struct msgbuf msg_to_send;
    printf("[S] [INIT] New client: %d\n", received_msg.client_id);

    int client_id = 0;
    while (client_id < MAX_CLIENTS && CLIENTS_QUEUE[client_id] != -1) client_id++;
    if (client_id >= MAX_CLIENTS) {
        printf(RED "[S] [ERROR] Max size!" RESET);
        return;
    }
    CLIENTS_QUEUE[client_id] = received_msg.client_id;

    msg_to_send.mtype = SERVER_INIT;
    msg_to_send.client_id = client_id;

    int snd;
    snd = msgsnd(received_msg.client_id, &msg_to_send, MSG_BUFF_SIZE, IPC_NOWAIT);
    if (snd < 0)
        printf(RED "[S] [ERROR] Problem with sending message\n" RESET);
    else
        printf("[S] [INIT] Message sent to client[%d]\n", client_id);
}

void handle_mirror(struct msgbuf msg) {
    printf("[S] [MIRROR] New message received: %d\n", msg.client_id);

    if (CLIENTS_QUEUE[msg.client_id] == -1) {
        printf("[S] [ERROR] Client not registered\n");
        return;
    }
    int client_queue_id = CLIENTS_QUEUE[msg.client_id];
    reverse(msg.text);

    int snd;
    snd = msgsnd(client_queue_id, &msg, MSG_BUFF_SIZE, IPC_NOWAIT);
    if (snd < 0)
        printf("[S] [ERROR] Problem with sending message\n");
    else
        printf("[S] [MIRROR] Message sent to client[%d]\n", msg.client_id);
}

void handle_time(struct msgbuf msg) {
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
    snd = msgsnd(client_queue_id, &msg, MSG_BUFF_SIZE, IPC_NOWAIT);
    if (snd < 0)
        printf("[S] [ERROR] Problem with sending message\n");
    else
        printf("[S] [TIME] Message sent to client[%d]\n", msg.client_id);
}

void handle_stop(struct msgbuf msg) {
    printf("[S] [STOP] New message received from %d\n", msg.client_id);
    if (CLIENTS_QUEUE[msg.client_id] == -1) {
        printf("[S] [ERROR] Client not registered\n");
        return;
    }
    int private_queue_id = CLIENTS_QUEUE[msg.client_id];
    msgctl(private_queue_id, IPC_RMID, NULL);
    CLIENTS_QUEUE[msg.client_id] = -1;
}

void reveive() {
    struct msgbuf received_msg;
    struct msqid_ds stat;
    bool server_end = false;
    msgqnum_t how_many = 1;
    while (how_many > 0) {
        ssize_t size;
        size = msgrcv(COMMON_QUEUE, &received_msg, sizeof(received_msg), 0, 0);
        if (size < 0)
            printf("[S] [ERROR] Receiving failed");

        switch (received_msg.mtype) {
            case SERVER_INIT:
                handle_new(received_msg);
                break;
            case SERVER_MIRROR:
                handle_mirror(received_msg);
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
                msgctl(COMMON_QUEUE, IPC_STAT, &stat);
                how_many = stat.msg_qnum + 1;
                server_end = true;
                break;
            default:
                printf("[S] [ERROR] Unknown message\n");
        }
        if (server_end) how_many--;
    }
    stop_handler(0);
}

void handle_math(struct msgbuf msgbuf) {
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
    snd = msgsnd(client_queue_id, &msgbuf, MSG_BUFF_SIZE, IPC_NOWAIT);
    if (snd < 0)
        printf("[S] [ERROR] Problem with sending message\n");
    else
        printf("[S] [MATH] Message sent to client[%d]\n", msgbuf.client_id);
}

int main(int argc, char **argv) {
    //create common queue
    key_t common_key = get_common_key();

    COMMON_QUEUE = msgget(common_key, IPC_CREAT | 0660);
    IF(COMMON_QUEUE < 0, "[C] Problem with creating common queue");

    printf("[S] Common queue opened with ID[%d]\n", COMMON_QUEUE);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        CLIENTS_QUEUE[i] = -1;
    }

    signal(SIGINT, stop_handler);
    reveive();

    return 0;
}