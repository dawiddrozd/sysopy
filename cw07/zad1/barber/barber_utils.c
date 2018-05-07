#define _GNU_SOURCE

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/sem.h>
#include "../common/common_utils.h"
#include "barber_utils.h"

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