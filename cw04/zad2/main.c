#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <time.h>
#include <stdbool.h>
#include <wait.h>

#define RED     "\x1b[91m"
#define CYAN    "\x1b[36m"
#define BLUE    "\x1b[34m"
#define GREEN   "\x1b[32m"
#define MAGNETA "\x1b[35m"
#define RESET   "\x1b[0m"

int expected_nr_of_requests = -1;
int *pending_processes = NULL;
int nr_of_pending = 0;
int nr_of_children = 0;
int nr_of_children_counter = 0;
pid_t *children_pids = NULL;

int PASS_EVERYONE = false;

siginfo_t *all_signals = NULL;
int nr_of_signals = 0;

//HOW MANY
int CONTINUE_COUNT = 0;
int SENDING_COUNT = 0;
int RECEIVED_COUNT = 0;
int DONE_COUNT = 0;
int RT_RECEIVED_COUNT = 0;

//FLAGS
bool CONTINUE_FLAG = true;
bool SENDING_FLAG = true;
bool RECEIVED_FLAG = true;
bool DONE_FLAG = true;
bool RT_RECEIVED_FLAG = true;
bool CREATED_FLAG = false;

void exit_properly();

static void sa_sigaction_handler(int sig, siginfo_t *siginfo, void *context) {
    if (sig == SIGINT) {
        exit_properly();
    }
    all_signals[nr_of_signals++] = *siginfo;
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

void exit_properly() {
    for (int i = 0; i < nr_of_children; i++) {
        int nr_of_killed = 0;
        if(children_pids[i] > 0) {
            kill(children_pids[i], SIGKILL);
            if(CREATED_FLAG)
                printf("[%d] was killed\n", children_pids[i]);
        }
    }
    free(children_pids);
    free(all_signals);
    exit(0);
}

pid_t f(siginfo_t *siginfo) {
    int status;
    pid_t pid = waitpid(siginfo->si_pid, &status, 0);
    if (pid != -1) {
        if (DONE_FLAG)
            printf(CYAN"[%d] Child process [%d] done with exit code %d.\n"RESET, ++DONE_COUNT, siginfo->si_pid,
                   WEXITSTATUS(status));
        nr_of_children_counter--;

        for (int i = 0; i < nr_of_children; i++) {
            if(children_pids[i] == pid) {
                children_pids[i] = -1;
                break;
            }
        }

        if (nr_of_children_counter == 0) {
            printf(BLUE "[DONE] All children are done. Program exited successfully.\n" RESET);
            exit_properly();
        }
    }
    return pid;
}

void handle_signal(siginfo_t *siginfo) {
    int sig = siginfo->si_signo;
    if (sig == SIGUSR1 && PASS_EVERYONE == false) {
        if (RECEIVED_FLAG)
            printf(RED "[%d] SIGUSR1 received from %d\n" RESET, ++RECEIVED_COUNT, siginfo->si_pid);

        pending_processes[nr_of_pending] = siginfo->si_pid;
        nr_of_pending++;

        if (nr_of_pending >= expected_nr_of_requests) {
            for (int i = 0; i < nr_of_pending; i++) {
                if (CONTINUE_FLAG)
                    printf(GREEN "[%d] Sending CONTINUE to %d\n" RESET, ++CONTINUE_COUNT, pending_processes[i]);
                kill(pending_processes[i], SIGCONT);
                sleep(1);
            }
            PASS_EVERYONE = true;
        }
    } else if (sig == SIGUSR1 && PASS_EVERYONE == true) {
        if (RECEIVED_FLAG)
            printf(RED "[%d] SIGUSR1 received from %d\n" RESET, ++RECEIVED_COUNT, siginfo->si_pid);
        if (CONTINUE_FLAG)
            printf(GREEN "[%d] Sending CONTINUE to %d\n" RESET, ++CONTINUE_COUNT, siginfo->si_pid);
        kill(siginfo->si_pid, SIGCONT);
    } else if (sig >= SIGRTMIN && sig <= SIGRTMAX) {
        if (RT_RECEIVED_FLAG)
            printf(MAGNETA "[%d] REALTIME%d received from %d\n" RESET, ++RT_RECEIVED_COUNT, sig, siginfo->si_pid);

        if (f(siginfo) < 0) {
            printf(RED "!!! Can't access PID = %d \n" RESET, siginfo->si_pid);
        }
    }
}

void do_child_job() {
    int useconds;
    srand((unsigned int) (time(NULL) ^ getpid()));
    useconds = rand() % 10000000;
    usleep((unsigned int) useconds);

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGCONT);

    void send_rt(int signo) {
        kill(getppid(), SIGRTMIN + rand() % (SIGRTMAX - SIGRTMIN));
        exit(useconds / 1000000);
    };

    if (signal(SIGCONT, send_rt) == SIG_ERR) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), "Can't set SIGINT");
    }

    if (SENDING_FLAG)
        printf(BLUE"[  ] SIGUSR1 sending from %d\n" RESET, getpid());

    kill(getppid(), SIGUSR1); //send request

    sigsuspend(&set); //wait for SIGCONT
}


int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Two int arguments are expected!\n");
        exit(EXIT_FAILURE);
    }
    nr_of_children = parse_string(argv[1]);
    nr_of_children_counter = nr_of_children;
    expected_nr_of_requests = parse_string(argv[2]);
    pending_processes = calloc((size_t) nr_of_children, sizeof(int));
    children_pids = calloc((size_t) nr_of_children, sizeof(int));

    for (int i = 0; i < nr_of_children; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Error: %s: %s\n", strerror(errno), "Fork failed");
            exit(errno);
        } else if (pid == 0) { //child
            do_child_job();
        } else {
            children_pids[i] = pid;
            if (CREATED_FLAG)
                printf("[%d] Process [%d] created\n", i + 1, pid);
        }
    }

    struct sigaction act;
    act.sa_handler = NULL;
    act.sa_sigaction = sa_sigaction_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGINT, &act, NULL);

    for (int sig = SIGRTMIN; sig <= SIGRTMAX; ++sig) {
        sigaction(sig, &act, NULL);
    }

    int nr_of_handled_signals = 0;
    all_signals = calloc((size_t) (nr_of_children * 10), sizeof(siginfo_t));
    while (1) {
        if (nr_of_handled_signals < nr_of_signals) {
            handle_signal(&all_signals[nr_of_handled_signals]);
            nr_of_handled_signals++;
        }
    }
}