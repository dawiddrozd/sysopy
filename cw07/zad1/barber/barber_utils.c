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
    barber->client_status = NONE;
    barber->clients_waiting = 0;
    barber->current_client = -1;
    barber->queue_head = 0;
    barber->queue_tail = 0;
    barber->nr_seats = nr_seats;
    for(int i=0; i<MAX_NR_CLIENTS; i++)
        barber->queue[i] = -1;
}

void handle_client(Barbershop *barbershop, int sem_id) {
    barbershop->client_status = INVITED;
    inc_sem(sem_id);
    while (barbershop->client_status != SITTING);
    dec_sem(sem_id);
    barbershop->barber_status = BUSY;
    printf(GREEN "[BARBER] I started shaving [#%d]\n" RESET, barbershop->current_client);
}

void run(int nr_seats, Barbershop *barbershop, int sem_id) {
    bool running = true;

    while (running) {
        dec_sem(sem_id);
        switch (barbershop->barber_status) {
            case AWOKEN:
                printf(GREEN "[BARBER] I was sleeping. Please come [#%d].\n" RESET, barbershop->current_client);
                handle_client(barbershop, sem_id);
                break;
            case BUSY:
                barbershop->client_status = LEAVING;
                printf(BLUE "[BARBER] Client #%d was shaved.\n" RESET, barbershop->current_client);
                barbershop->barber_status = FREE;
                break;
            case FREE:
                if(barbershop->clients_waiting > 0) {
                    barbershop->current_client = queue_pop(barbershop);
                    barbershop->client_status = INVITED;
                    printf(GREEN "[BARBER] I finished a shaving but still have clients, please come [#%d].\n" RESET, barbershop->current_client);
                    handle_client(barbershop, sem_id);
                } else {
                    printf(BLUE "[Barber] No one's is here. Let's take a nap.\n" RESET);
                    barbershop->barber_status = SLEEPING;
                }
                break;
            case SLEEPING:
                break;
        }
        inc_sem(sem_id);
    }
}