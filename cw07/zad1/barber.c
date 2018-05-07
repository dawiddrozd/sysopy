#define _GNU_SOURCE

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <signal.h>
#include "../common/common_utils.h"
#include "barber_utils.h"

int sem_id;
int shm_id;

void stop_handler(int signum) {
    printf(MAG "[Barber] Shop closed. Goodbye!\n" RESET);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    exit(0);
}

pid_t queue_pop(Barbershop *barbershop) {
    if (barbershop->clients_waiting > 0) {
        int head = barbershop->queue_head;
        int result = barbershop->queue[head];
        barbershop->queue[head] = -1;
        barbershop->queue_head = (barbershop->queue_head + 1) % MAX_NR_CLIENTS;
        barbershop->clients_waiting--;
        return result;
    } else {
        return -1;
    }
}

void barber_init(int nr_seats, Barbershop *barber) {
    if (barber == NULL) {
        return;
    }
    barber->barber_status = FREE;
    barber->clients_waiting = 0;
    barber->current_client = 0;
    barber->queue_head = 0;
    barber->queue_tail = 0;
    barber->nr_seats = nr_seats;
    for (int i = 0; i < MAX_NR_CLIENTS; i++)
        barber->queue[i] = 0;
}

void run(int nr_seats, Barbershop *barbershop, int sem_id) {
    bool running = true;

    while (running) {
        inc_sem(sem_id);
        switch (barbershop->barber_status) {
            case AWOKEN:
                if(barbershop->current_client != 0){
                    printf(GREEN "[BARBER] I was sleeping. Please come [#%d].\n" RESET, barbershop->current_client);
                    barbershop->barber_status = BUSY;
                } else {
                    barbershop->barber_status = FREE;
                }
                break;
            case BUSY:
                printf(GREEN "[BARBER] I started shaving [#%d]\n" RESET, barbershop->current_client);
                printf(GREEN "[BARBER] I finished shaving [#%d]\n" RESET, barbershop->current_client);
                barbershop->current_client = 0;
                barbershop->barber_status = FREE;
                break;
            case FREE:
                if (barbershop->clients_waiting > 0) {
                    //invite client
                    barbershop->current_client = queue_pop(barbershop);
                    printf(GREEN "[BARBER] Please come [#%d].\n" RESET, barbershop->current_client);
                    barbershop->barber_status = BUSY;
                } else {
                    printf(BLUE "[BARBER] No one's is here. Let's take a nap.\n" RESET);
                    barbershop->barber_status = SLEEPING;
                    barbershop->current_client = 0;
                }
                break;
            case SLEEPING:
                break;
        }
        dec_sem(sem_id);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "One argument expected\n");
        exit(1);
    }

    int nr_seats = to_int(argv[1]);
    check_exit(nr_seats < 0, "Only numbers expected as first argument");

    // init shared memory
    key_t shm_key;
    shm_key = ftok(get_homedir(), SHM_CHAR);
    check_exit(shm_key < 0, "Ftok failed.");
    shm_id = shmget(shm_key, sizeof(Barbershop), IPC_CREAT | 0666);
    check_exit(shm_id < 0, "Shmget failed.");
    Barbershop *barber_shop;
    barber_shop = shmat(shm_id, NULL, 0);
    check_exit(barber_shop == (void *) -1, "Shmat failed");


    // init semaphore
    key_t sem_key;
    sem_key = ftok(get_homedir(), SEM_CHAR);
    check_exit(sem_key < 0, "Ftok failed.");
    sem_id = semget(sem_key, 1, IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO);
    check_exit(sem_id < 0, "Semget failed.");
    semctl(sem_id, 0, SETVAL, 1); //don't know if works!!
    //inc_sem(sem_id);

    barber_init(nr_seats, barber_shop);
    signal(SIGINT, stop_handler);

    run(nr_seats, barber_shop, sem_id);

    return 0;
}