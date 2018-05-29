#define _XOPEN_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>

#define EXPANDED_TYPE 1
#define SMALLER 1
#define EQUAL 2
#define GREATER 3
#define NAME_1 "/s1"
#define NAME_2 "/s2"
#define NAME_3 "/s3"
#define NAME_4 "/s4"
#define NAME_5 "/s5"

void sem_give(sem_t *semaphore) {
    if (sem_post(semaphore) == -1) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), "Incrementing semaphore failed.");
    }
}

void sem_take(sem_t *semaphore) {
    if (sem_wait(semaphore) < 0) {
        printf("%s", strerror(errno));
    }
}

int MAX_LINE_LENGTH = 128;
int NR_PRODUCERS = 10;
int NR_CONSUMERS = 10;
int BUFFER_SIZE = 128;
int EXPECTED_LENTGH = 0;
int PRINTF_TYPE = 0; //tryb standard(0)/expanded(1)
int COMPARING_TYPE = 0;
int NK = 0; //liczba sekund do końca
char **ARRAY = NULL;
FILE *FILE_IN = NULL;

int lines_read = 0;
int last_idx_read = 0;
int last_idx_write = 0;
pthread_t *producers;
pthread_t *consumers;

int counter = 0;
sem_t *sem_read = NULL;
sem_t *sem_mutex = NULL;
sem_t *sem_printf = NULL;
sem_t *sem_fillCount = NULL;
sem_t *sem_emptyCount = NULL;

int reading_end = 0;

void check_exit(int correct, const char *message) {
    if (correct) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), message);
        exit(errno);
    }
}

void alarm_handler(int sig) {
    printf("PROGRAM EXIT!\n");
    reading_end = 1;
}

void global_exit_handler() {
    sem_unlink(NAME_1);
    sem_unlink(NAME_2);
    sem_unlink(NAME_3);
    sem_unlink(NAME_4);
    sem_unlink(NAME_5);
    free(ARRAY);
    fclose(FILE_IN);
}

void parse_arguments(int argc, char **argv) {
    if (argc < 2) {
        perror("One argument expected!\n");
        exit(0);
    }
    char *filename = argv[1];
    FILE *stream = NULL;
    if ((stream = fopen(filename, "r")) == NULL) {
        perror("Can't open conf file.\n");
        exit(0);
    }
    fscanf(stream, "%d", &NR_PRODUCERS);
    fscanf(stream, "%d", &NR_CONSUMERS);
    fscanf(stream, "%d", &BUFFER_SIZE);

    char buffer[MAX_LINE_LENGTH];
    fscanf(stream, "%s", buffer);
    fscanf(stream, "%d", &EXPECTED_LENTGH);
    fscanf(stream, "%d", &PRINTF_TYPE);
    fscanf(stream, "%d", &NK);
    fscanf(stream, "%d", &COMPARING_TYPE);

    if ((FILE_IN = fopen(buffer, "r")) == NULL) {
        perror("Can't open input file.\n");
        exit(0);
    }
}

typedef struct Item {
    char *line;
    int idx;
} Item;

void consumeItem(Item *item) {
    size_t length = strlen(item->line);
    int should_print = 0;
    switch (COMPARING_TYPE) {
        case SMALLER:
            if (length < EXPECTED_LENTGH) {
                should_print = 1;
            }
            break;
        case EQUAL:
            if (length == EXPECTED_LENTGH) {
                should_print = 1;
            }
            break;
        case GREATER:
            if (length > EXPECTED_LENTGH) {
                should_print = 1;
            }
            break;
        default:
            break;
    }
    if (should_print) {
        sem_take(sem_printf);
        printf("[CONSUMER][%d][%d]: %s", item->idx, ++lines_read, item->line);
        sem_give(sem_printf);
    }
}

Item *removeItemFromBuffer() {
    Item *item = malloc(sizeof(Item));
    item->idx = -1;
    item->line = NULL;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        int idx = (last_idx_read + i) % BUFFER_SIZE;
        if (ARRAY[idx] != NULL) {
            item->idx = idx;
            item->line = ARRAY[idx];
            last_idx_read = idx;
            ARRAY[idx] = NULL;
            counter--;
            break;
        }
    }
    assert(item->line != NULL);
    if (PRINTF_TYPE == EXPANDED_TYPE) {
        sem_take(sem_printf);
        printf("[CONSUMER][%d]: Removing line from buffer: %s", item->idx, item->line);
        sem_give(sem_printf);
    }
    return item;
}

void putLineIntoBuffer(char *line) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        int idx = (last_idx_write + i) % BUFFER_SIZE;
        if (ARRAY[idx] == NULL) {
            ARRAY[idx] = malloc((strlen(line) + 1) * sizeof(char));
            strcpy(ARRAY[idx], line);
            free(line);
            last_idx_write = idx;
            counter++;
            break;
        }
    }
    if (PRINTF_TYPE == EXPANDED_TYPE) {
        sem_take(sem_printf);
        printf("[PRODUCER][%d]: Adding line to buffer: %s", last_idx_write, line);
        sem_give(sem_printf);
    }
}

char *produceLine() {
    char *line = malloc(MAX_LINE_LENGTH * sizeof(char));
    if (FILE_IN != NULL) {
        size_t len = (size_t) MAX_LINE_LENGTH;
        if (getline(&line, &len, FILE_IN) == -1) {
            if (NK == 0) {
                reading_end = 1;
            }
            sem_give(sem_read);
            pthread_exit((void *) 0);
        }
        line[strlen(line)] = '\0';
    }
    return line;
}

void *producer(void *param) {
    while (!reading_end) {
        sem_take(sem_read);
        char *line = produceLine();
        sem_give(sem_read);

        sem_take(sem_emptyCount);
        sem_take(sem_mutex);
        putLineIntoBuffer(line);
        sem_give(sem_mutex);
        sem_give(sem_fillCount);
    }
    pthread_exit((void *) 0);
}

void *consumer(void *param) {
    while (1) {
        sem_take(sem_fillCount);
        sem_take(sem_mutex);
        Item *item = removeItemFromBuffer();
        sem_give(sem_mutex);
        sem_give(sem_emptyCount);
        consumeItem(item);
        free(item->line);
        free(item);
    }
}

void threads_start() {
    producers = malloc(NR_PRODUCERS * sizeof(pthread_t));
    consumers = malloc(NR_CONSUMERS * sizeof(pthread_t));
    for (int i = 0; i < NR_PRODUCERS; i++) {
        pthread_create(&producers[i], NULL, producer, NULL);
    }
    for (int i = 0; i < NR_CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer, NULL);
    }

    struct sigaction *action = &(struct sigaction) {.sa_handler = alarm_handler};

    if (NK > 0) {
        sigaction(SIGALRM, action, NULL);
        alarm((unsigned int) NK);
    } else if (NK == 0) {
        sigaction(SIGINT, action, NULL);
    }

    for (int i = 0; i < NR_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }

    while(1) {
        sem_take(sem_mutex);
        if(counter == 0) break;
        sem_give(sem_mutex);
    }

    free(producers);
    free(consumers);
}

void initialize_sem(sem_t **sem, int init_value, char *path) {

    *sem = sem_open(path, O_CREAT | O_EXCL, 0666, init_value);
    check_exit(*sem == SEM_FAILED, "Can't create semaphore");
}

int main(int argc, char **argv) {
    parse_arguments(argc, argv);
    ARRAY = (char **) malloc(BUFFER_SIZE * sizeof(char *));
    atexit(global_exit_handler);

    initialize_sem(&sem_mutex, 1, NAME_1);
    initialize_sem(&sem_printf, 1, NAME_2);
    initialize_sem(&sem_read, 1, NAME_3);
    initialize_sem(&sem_emptyCount, BUFFER_SIZE, NAME_4);
    initialize_sem(&sem_fillCount, 0, NAME_5);

    threads_start();

    return 0;
}