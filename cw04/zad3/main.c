#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <bits/signum.h>
#include <signal.h>
#include <wait.h>

#define BLUE    "\x1b[34m"
#define GREEN   "\x1b[32m"
#define RESET   "\x1b[0m"

int PARENT_SIGNALS_SENT = 0;
int PARENT_SIGNALS_RECEIVED = 0;
int CHILD_SIGNALS_RECEIVED = 0;

int CHILD_PID = -1;
int NR_OF_SIGNALS = -1;

void parent_handler(int sig, siginfo_t *siginfo, void *context) {
    if (sig == SIGUSR1 || sig == SIGRTMIN || sig == SIGRTMIN + 1) {
        printf(BLUE "[%d] SIG%d received from child.\n" RESET, ++PARENT_SIGNALS_RECEIVED, sig);
    } else if (sig == SIGINT) {
        kill(CHILD_PID, SIGUSR2);
        int status;
        waitpid(CHILD_PID, &status, 0);
    } else if (sig == SIGCONT) {
        printf(BLUE "[0] SIGCONT received from child. Work start.\n" RESET);
    } else if (sig == SIGCHLD) {
        printf(BLUE "[0] SIGCHLD received from child. Work end.\n" RESET);
        exit(0);
    }
}

void child_handler(int sig, siginfo_t *siginfo, void *context) {

    if (sig == SIGUSR1 || sig == SIGRTMIN || sig == SIGRTMIN + 1) {
        printf(GREEN "[%d] SIG%d received from parent. \n" RESET, ++CHILD_SIGNALS_RECEIVED, sig);
        kill(getppid(), sig);
    } else if (sig == SIGUSR2) {
        exit(0);
    }
}

void block_all_except(int how_many, int *arr) {

    sigset_t set;
    sigfillset(&set);
    for (int i = 0; i < how_many; i++) {
        sigdelset(&set, arr[i]);
    }
    sigprocmask(SIG_SETMASK, &set, NULL);
}

void parent(int type, int L) {

    struct sigaction act;
    act.sa_handler = NULL;
    act.sa_sigaction = parent_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);
    sigaction(SIGCONT, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGRTMIN, &act, NULL);
    sigaction(SIGRTMIN + 1, &act, NULL);

    sleep(10); //wait for child
    sigset_t set;

    switch (type) {
        case 1:
            for (int i = 0; i < L; i++) {
                printf(BLUE "[%d] SIGUSR1 sent to child.\n" RESET, ++PARENT_SIGNALS_SENT);
                kill(CHILD_PID, SIGUSR1);
                sleep(1);
            }
            break;
        case 2:
            sigfillset(&set);
            sigdelset(&set, SIGUSR1);

            for (int i = 0; i < L; i++) {
                printf(BLUE "[%d] SIGUSR1 sent to child.\n" RESET, ++PARENT_SIGNALS_SENT);
                kill(CHILD_PID, SIGUSR1);
                sigsuspend(&set); //wait for SIGUSR1
            }
            break;
        case 3:
            for (int i = 0; i < L; i++) {
                printf(BLUE "[%d] SIGRT%d sent to child.\n" RESET, ++PARENT_SIGNALS_SENT, SIGRTMIN + i % 2);
                kill(CHILD_PID, SIGRTMIN + i % 2);
                sleep(1);
            }
            break;
        default:
            fprintf(stderr, "Expected values for type argument: 1 2 3\n");
            exit(EXIT_FAILURE);
    }

    kill(CHILD_PID, SIGUSR2);
    while (1)
        sleep(10);
}

void child() {

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigdelset(&set, SIGUSR2);
    sigdelset(&set, SIGRTMIN);
    sigdelset(&set, SIGRTMIN + 1);
    sigprocmask(SIG_SETMASK, &set, NULL);

    struct sigaction act;
    act.sa_handler = NULL;
    act.sa_sigaction = child_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR1, &act, NULL) == -1 ||
        sigaction(SIGUSR2, &act, NULL) == -1 ||
        sigaction(SIGRTMIN, &act, NULL) == -1 ||
        sigaction(SIGRTMIN + 1, &act, NULL) == -1) {
        fprintf(stderr, "Can't set child handler.");
    }

    kill(getppid(), SIGCONT);

    while (1)
        sleep(10);
}

int parse_string(char *string) {
    char *errptr;
    int number = (int) strtol(string, &errptr, 10);
    if (*errptr != '\0') {
        fprintf(stderr, "Can't parse second argument: Only numbers are expected.\n");
        exit(EXIT_FAILURE);
    }
    return number;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Two arguments expected.\n");
        exit(EXIT_FAILURE);
    }
    int nr_of_signals = parse_string(argv[1]);

    int type = parse_string(argv[2]);
    if (type < 1 || type > 3) {
        fprintf(stderr, "Expected values for second argument: 1 2 3\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == 0) {
        child();
    } else if (pid == -1) {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        exit(errno);
    } else {
        CHILD_PID = pid;
        parent(type, nr_of_signals);
    }
}