#define _GNU_SOURCE
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "../common/common_utils.h"
#include "client_utils.h"

int main(int argc, char **argv) {
    check_exit(argc < 3, "Two arguments expected");

    int nr_clients = to_int(argv[1]);
    int nr_trimming = to_int(argv[2]);
    check_exit(nr_clients < 0 || nr_trimming < 0, "Can't parse arguments");

    //open semaphore
    key_t sem_key;
    sem_key = ftok(get_homedir(), SEM_CHAR);
    check_exit(sem_key < 0, "Ftok failed.");
    int sem_id;
    sem_id = semget(sem_key, 0, 0);
    check_exit(sem_id < 0, "Semget failed.");
    semctl(sem_id, 0, SETVAL, 1);

    //open shared memory
    key_t shm_key;
    shm_key = ftok(get_homedir(), SHM_CHAR);
    check_exit(shm_key < 0, "Ftok failed.");
    int shm_id;
    shm_id = shmget(shm_key, 0, 0);
    check_exit(shm_id < 0, "Shmget failed.");
    Barbershop *barber_shop;
    barber_shop = shmat(shm_id, NULL, 0);
    check_exit(barber_shop == (void *) -1, "Shmat failed");

    run(nr_clients, nr_trimming, barber_shop, sem_id);

    return 0;
}