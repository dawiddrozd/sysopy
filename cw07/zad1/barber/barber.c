#define _GNU_SOURCE

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <bits/signum.h>
#include <signal.h>
#include "../common/common_utils.h"
#include "barber_utils.h"

int sem_id;
int shm_id;

void stop_handler(int signum) {
    printf(MAG "[Barber] Shop closed. Goodbye!\n" RESET);
    shmctl(shm_id, IPC_RMID, NULL);
    exit(0);
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