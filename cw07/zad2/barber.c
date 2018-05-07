#define _GNU_SOURCE

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include "common.h"

sem_t* semaphore;
int shared_mem_id;

void stop_handler(int signum) {
    if(semaphore != NULL) {
        sem_close(semaphore);
    }
    if(shared_mem_id != 0) {
        close(shared_mem_id);
        shm_unlink(SEM_PATH);
    }
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
    barber->barber_status = SLEEPING;
    barber->clients_waiting = 0;
    barber->current_client = 0;
    barber->current_client_status = NONE;
    barber->queue_head = 0;
    barber->queue_tail = 0;
    barber->nr_seats = nr_seats;
    for (int i = 0; i < MAX_NR_CLIENTS; i++)
        barber->queue[i] = 0;
}

void run(int nr_seats, Barbershop *barbershop, sem_t *sem) {
    bool running = true;

    char buffer[DATE_SIZE];
    while (running) {
        sem_take(sem);
        switch (barbershop->barber_status) {
            case AWOKEN:
                if (barbershop->current_client != 0) {
                    printf(GREEN "[%s][BARBER] I was sleeping. Please come [#%d].\n" RESET, gettime(buffer), barbershop->current_client);
                    barbershop->barber_status = BUSY;
                } else {
                    barbershop->barber_status = FREE;
                }
                break;
            case BUSY:
                sem_give(sem);
                while (barbershop->current_client_status != SITTING);
                printf(GREEN "[%s][BARBER] I started shaving [#%d]\n" RESET, gettime(buffer), barbershop->current_client);
                sem_take(sem);
                printf(GREEN "[%s][BARBER] I finished shaving [#%d]\n" RESET, gettime(buffer), barbershop->current_client);
                barbershop->barber_status = FREE;
                barbershop->current_client = 0;
                barbershop->current_client_status = NONE;
                break;
            case FREE:
                if (barbershop->clients_waiting > 0) {
                    //invite client
                    barbershop->current_client = queue_pop(barbershop);
                    printf(GREEN "[%s][BARBER] Please come [#%d].\n" RESET, gettime(buffer), barbershop->current_client);
                    barbershop->barber_status = BUSY;
                } else {
                    printf(BLUE "[%s][BARBER] No one's is here. Let's take a nap.\n" RESET,  gettime(buffer));
                    barbershop->barber_status = SLEEPING;
                    barbershop->current_client = 0;
                    barbershop->current_client_status = NONE;
                }
                break;
            case SLEEPING:
                break;
        }
        sem_give(sem);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "One argument expected\n");
        exit(1);
    }

    int nr_seats = to_int(argv[1]);
    check_exit(nr_seats < 0, "Only numbers expected as first argument");

    // init semaphore
    semaphore = sem_open(SEM_PATH, O_CREAT | O_RDWR, 0666, 1);
    check_exit(semaphore == (sem_t*) -1, "Can't create semaphore");

    // init shared memory
    shared_mem_id = shm_open(SHM_PATH, O_RDWR | O_CREAT, S_IRWXU);
    check_exit(shared_mem_id == -1, "Can't create shared memory");
    ftruncate(shared_mem_id, sizeof(struct Barbershop));

    Barbershop *barber_shop;
    barber_shop = mmap(NULL, sizeof(struct Barbershop), PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_id, 0);
    check_exit(barber_shop == (void *) -1, "Can't access shared memory");

    barber_init(nr_seats, barber_shop);
    signal(SIGINT, stop_handler);

    run(nr_seats, barber_shop, semaphore);

    return 0;
}