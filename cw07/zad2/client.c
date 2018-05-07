#define _GNU_SOURCE
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/shm.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include "common.h"

sem_t* semaphore;
int shared_mem_id;

void queue_add(pid_t client, Barbershop *barbershop) {
    barbershop->queue_tail = barbershop->queue_tail % MAX_NR_CLIENTS;
    barbershop->queue[barbershop->queue_tail] = client;
    barbershop->queue_tail++;
    barbershop->clients_waiting++;
}

int sem_getval(int sem_id) {
    return semctl(sem_id, 0, GETVAL, 0);
}

void run(int nr_clients, int nr_trimming, Barbershop *barbershop, sem_t *sem) {
    int pid = -1;
    for (int i = 0; i < nr_clients; i++) {
        pid = fork();
        if (pid == 0) break;
    }

    char buffer[DATE_SIZE];
    //child
    if (pid == 0) {
        while (nr_trimming) {
            enum client_status my_status = NONE;
            // when i enter
            sem_take(sem);
            switch (barbershop->barber_status) {
                case SLEEPING:
                    printf(GREEN "[%s][Client #%d] Let's wake up a barber! \n" RESET, gettime(buffer), getpid());
                    barbershop->barber_status = AWOKEN;
                    barbershop->current_client = getpid();
                    my_status = WAITING;
                    break;
                case FREE:
                case AWOKEN:
                case BUSY:
                    if (barbershop->clients_waiting < barbershop->nr_seats) {
                        printf(BLUE "[%s][Client #%d] Waiting in queue.\n" RESET, gettime(buffer), getpid());
                        queue_add(getpid(), barbershop);
                        my_status = WAITING;
                    } else {
                        printf(BLUE "[%s][Client #%d] No more free space. I leave.\n" RESET, gettime(buffer), getpid());
                        sem_give(sem);
                        exit(0);
                    }
                    break;
                default:
                    break;
            }
            sem_give(sem);

            if (my_status == WAITING) {

                while (my_status == WAITING) {
                    sem_take(sem);
                    if (barbershop->current_client == getpid()) {
                        my_status = INVITED;
                        barbershop->current_client_status = SITTING;
                        printf(GREEN "[%s][Client #%d] I was invited.\n" RESET, gettime(buffer), getpid());
                    }
                    sem_give(sem);
                }

                while (my_status == INVITED) {
                    sem_take(sem);
                    if (barbershop->current_client != getpid()) {
                        my_status = NONE;
                        printf(MAG "[%s][Client #%d] I was shaved. Haircut left: [%d]\n" RESET, gettime(buffer), getpid(), --nr_trimming);
                    }
                    sem_give(sem);
                }
            }
        }
    }

    //parent
    if (pid > 0) {
        for (int i = 0; i < nr_clients; i++) {
            wait(NULL);
        }
    }
}

int main(int argc, char **argv) {
    check_exit(argc < 3, "Two arguments expected");

    int nr_clients = to_int(argv[1]);
    int nr_trimming = to_int(argv[2]);
    check_exit(nr_clients < 0 || nr_trimming < 0, "Can't parse arguments");

    //open shared memory
    shared_mem_id = shm_open(SHM_PATH, O_RDWR, S_IRWXU);
    check_exit(shared_mem_id == -1, "Can't create shared memory");
    ftruncate(shared_mem_id, sizeof(struct Barbershop));
    Barbershop *barber_shop;
    barber_shop = mmap(NULL, sizeof(struct Barbershop), PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_id, 0);
    check_exit(barber_shop == (void *) -1, "Can't access shared memory");

    //open semaphore
    semaphore = sem_open(SEM_PATH, 0);
    check_exit(semaphore == (sem_t*) -1, "Can't create semaphore");

    run(nr_clients, nr_trimming, barber_shop, semaphore);

    return 0;
}