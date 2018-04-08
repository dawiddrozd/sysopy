#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <memory.h>

pid_t CHILD_PID = -1;

static void sigstp_pause(int signo);

static void run() {
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
                // printf("Process with PID = %d started!\n", getpid());
                signal(SIGINT, SIG_IGN);
                signal(SIGTSTP, SIG_IGN);
                execlp("date", "date", (char *) NULL);
            }
            int status;
            waitpid(pid_for, &status, (int) NULL);
            sleep(1);
        }
    } else {
        pid_t status;
        CHILD_PID = pid;
    }
}

static void sig_usr(int signo) {
    if (signo == SIGINT) {
        printf(": Odebrano sygnał SIGINT\n");
        if (CHILD_PID > 0) {
            kill(CHILD_PID, SIGKILL);
            CHILD_PID = -1;
        }
        exit(0);
    }
}

static void sigtstp_running(int signo) {
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

static void sigstp_pause(int signo) {
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