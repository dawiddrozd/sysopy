#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <stdbool.h>
#include "../common/utils.h"

int MY_ID = -1;
int PUBLIC_QUEUE_ID = -1;
int PRIVATE_QUEUE_ID = -1;

void stop_handler(int signum) {
    printf("CLIENT STOPPED!\n");
    struct msgbuf send_msg;
    send_msg.client_id = MY_ID;
    printf(GREEN "[C] SERVER END command\n" RESET);
    int snd = msgsnd(PUBLIC_QUEUE_ID, &send_msg, MSG_BUFF_SIZE, IPC_NOWAIT);
    IF(snd < 0, "[C] Problem with sending mirror message");
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

void send_and_receive(struct msgbuf *send_msg,
                      struct msgbuf *received_msg) {
    int snd;
    snd = msgsnd(PUBLIC_QUEUE_ID, send_msg, MSG_BUFF_SIZE, IPC_NOWAIT);
    IF(snd < 0, "[C] Problem with sending message");
    ssize_t size;
    size = msgrcv(PRIVATE_QUEUE_ID, received_msg, MSG_BUFF_SIZE, 0, 0);
    IF(size < 0, "[S] Receiving failed");
}

void run(FILE *file) {
    struct msgbuf send_msg;
    struct msgbuf received_msg;
    char line[MAX_CMDS];

    bool running = true;
    while (running && fgets(line, MAX_BUFF_SIZE, file)) {
        char *p = strtok(line, " \t\n");
        send_msg.mtype = to_command(p);
        switch (send_msg.mtype) {
            case SERVER_INIT:
                printf(GREEN "[C] SERVER_INIT init command sending\n" RESET);
                send_msg.client_id = PRIVATE_QUEUE_ID;
                send_and_receive(&send_msg, &received_msg);
                MY_ID = received_msg.client_id;
                printf(BLUE "[C] SERVER_INIT message received from server ID[%d]\n" RESET, received_msg.client_id);
                break;
            case SERVER_MIRROR:
                printf(GREEN "[C] SERVER_MIRROR command sending: %s\n" RESET, p);
                p = strtok(NULL, "\n");
                send_msg.client_id = received_msg.client_id;
                sprintf(send_msg.text, "%s", p);
                send_and_receive(&send_msg, &received_msg);
                printf(BLUE "[C] SERVER_MIRROR message received: %s\n" RESET, received_msg.text);
                break;
            case SERVER_TIME:
                printf(GREEN "[C] SERVER_TIME command sending: %s\n" RESET, p);
                send_msg.client_id = received_msg.client_id;
                send_and_receive(&send_msg, &received_msg);
                printf(BLUE "[C] SERVER_TIME message received: %s\n" RESET, received_msg.text);
                break;
            case SERVER_ADD:
            case SERVER_DIV:
            case SERVER_MUL:
            case SERVER_SUB:
                printf(GREEN "[C] SERVER MATH command sending: %s\n" RESET, p);
                send_msg.client_id = received_msg.client_id;
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
                send_msg.client_id = received_msg.client_id;
                printf(GREEN "[C] SERVER STOP/END command\n" RESET);
                int snd = msgsnd(PUBLIC_QUEUE_ID, &send_msg, MSG_BUFF_SIZE, IPC_NOWAIT);
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

    // open private and public queue
    key_t private_key = get_private_key();
    key_t common_key = get_common_key();

    int private_queue_id;
    private_queue_id = msgget(private_key, IPC_CREAT | 0660);
    IF(private_queue_id < 0, "[C] Problem with opening private queue");

    int public_queue_id;
    public_queue_id = msgget(common_key, 0660);
    IF(public_queue_id < 0, "[C] Problem with opening common queue");
    PUBLIC_QUEUE_ID = public_queue_id;
    PRIVATE_QUEUE_ID = private_queue_id;

    signal(SIGINT, stop_handler);
    run(file);

    return 0;
}