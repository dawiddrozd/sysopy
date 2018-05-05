#define _GNU_SOURCE
#ifndef SYSOPY4_BARBER_UTILS_H
#define SYSOPY4_BARBER_UTILS_H

#include "../common/common_utils.h"

void barber_init(int nr_seats, Barbershop *barber);
void run(int nr_seats, Barbershop *barbershop, int sem_id);

#endif //SYSOPY4_BARBER_UTILS_H
