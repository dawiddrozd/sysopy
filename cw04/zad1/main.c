#define _POSIX_SOURCE
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <memory.h>
#include <wait.h>

pid_t CHILD_PID = -1;

void sigstp_pause(int signo);

void run() {
    int pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), "Fork failed");
        exit(errno);
    } else if (pid == 0) { //child
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        for (;;) {
            int pid_for = fork();
            if (pid_for < 0) {
                fprintf(stderr, "Error: %s: %s\n", strerror(errno), "Fork in for loop failed");
                exit(errno);
            } else if (pid_for == 0) {
                signal(SIGINT, SIG_IGN);
                signal(SIGTSTP, SIG_IGN);
                execlp("date", "date", NULL);
            }
            int status;
            waitpid(pid_for, &status, 0);
            sleep(1);
        }
    } else {
        CHILD_PID = pid;
    }
}

void sig_usr(int signo) {
    if (signo == SIGINT) {
        printf(": Odebrano sygnał SIGINT\n");
        if (CHILD_PID > 0) {
            kill(CHILD_PID, SIGKILL);
            CHILD_PID = -1;
        }
        exit(0);
    }
}

void sigtstp_running(int signo) {
    if (signo == SIGTSTP) {
        if (CHILD_PID > 0) {
            kill(CHILD_PID, SIGKILL);
            CHILD_PID = -1;
        }
        printf(": Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
        struct sigaction act;
        act.sa_handler = sigstp_pause;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGTSTP, &act, NULL);
    }
}

void sigstp_pause(int signo) {
    if (signo == SIGTSTP) {
        // set proper handler
        struct sigaction act;
        act.sa_handler = sigtstp_running;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGTSTP, &act, NULL);

        //and run
        if (CHILD_PID < 0) {
            run();
        }
    }
}

int main() {
    if (signal(SIGINT, sig_usr) == SIG_ERR) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), "Can't set SIGINT");
        exit(errno);
    }

    struct sigaction act;
    act.sa_handler = sigtstp_running;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGTSTP, &act, NULL);

    run();

    while (1)
        sleep(10);
}