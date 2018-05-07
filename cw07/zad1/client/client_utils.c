#define _GNU_SOURCE

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include "client_utils.h"

void queue_add(pid_t client, Barbershop *barbershop) {
    barbershop->queue_tail = barbershop->queue_tail % MAX_NR_CLIENTS;
    barbershop->queue[barbershop->queue_tail] = client;
    barbershop->queue_tail++;
    barbershop->clients_waiting++;
}

void run(int nr_clients, int nr_trimming, Barbershop *barbershop, int sem_id) {
    int pid = -1;
    for (int i = 0; i < nr_clients; i++) {
        pid = fork();
        if (pid == 0) break;
    }

    //child
    if (pid == 0) {
        enum client_status my_status = NONE;
        while (nr_trimming) {
            // when i enter
            dec_sem(sem_id);
            switch (barbershop->barber_status) {
                case FREE:
                case AWOKEN:
                case BUSY:
                    if (barbershop->clients_waiting < barbershop->nr_seats) {
                        printf(BLUE "[Client #%d] I will sit and wait in queue\n" RESET, getpid());
                        queue_add(getpid(), barbershop);
                        my_status = WAITING;
                    } else {
                        printf(BLUE "[Client #%d] No more free space in waiting room. I leave.\n" RESET, getpid());
                        my_status = LEAVING;
                        //usleep(300);
                    }
                    break;
                case SLEEPING:
                    printf(GREEN "[Client #%d] Let's wake up a barber! \n" RESET, getpid());
                    barbershop->current_client = getpid();
                    barbershop->barber_status = AWOKEN;
                    my_status = WAITING;
                    break;
            }

            if (my_status == WAITING) {
                inc_sem(sem_id);
                while (barbershop->current_client != getpid());
                printf(GREEN "[Client #%d] I was invited.\n" RESET, getpid());
                dec_sem(sem_id);

                barbershop->client_status = SITTING;
                printf(GREEN "[Client #%d] Sitting on the armchair.\n" RESET, getpid());

                inc_sem(sem_id);
                while (barbershop->current_client == getpid());
                printf(MAG "[Client #%d] I was shaved. Haircut left: [%d]\n" RESET, getpid(), nr_trimming);
                dec_sem(sem_id);
                my_status = NONE;
                nr_trimming--;
            }

            inc_sem(sem_id);
        }

    }

    //parent
    if (pid > 0) {
        for (int i = 0; i < nr_clients; i++) {
            wait(NULL);
        }
    }
}