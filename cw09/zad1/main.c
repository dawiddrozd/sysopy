#include <pthread.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define EXPANDED_TYPE 1

#define SMALLER 1
#define EQUAL 2
#define GREATER 3

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
int item_count = 0;
int last_idx_read = 0;
int last_idx_write = 0;
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t read_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_end = PTHREAD_COND_INITIALIZER;
pthread_t *producers;
pthread_t *consumers;

int reading_end = 0;

void alarm_handler(int sig) {
    printf("PROGRAM EXIT!\n");
    reading_end = 1;
    item_count = 0;
}

void global_exit_handler() {
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
    pthread_mutex_lock(&printf_mutex);
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
        printf("[CONSUMER][%d][%d]: %s", item->idx, ++lines_read, item->line);
    }
    pthread_mutex_unlock(&printf_mutex);
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
            break;
        }
    }
    assert(item->line != NULL);
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
            break;
        }
    }
    if (PRINTF_TYPE == EXPANDED_TYPE) {
        pthread_mutex_lock(&printf_mutex);
        printf("[PRODUCER][%d]: Adding line to buffer: %s", last_idx_write, line);
        pthread_mutex_unlock(&printf_mutex);
    }
}

void add(char *line) {
    pthread_mutex_lock(&buffer_mutex);
    while (item_count == BUFFER_SIZE) {
        pthread_cond_wait(&cond_full, &buffer_mutex);
    }
    putLineIntoBuffer(line);
    item_count++;

    if (item_count == 1) {
        pthread_cond_broadcast(&cond_empty); //or broadcoast!!!
    }

    pthread_mutex_unlock(&buffer_mutex);
}

Item *_remove() {
    pthread_mutex_lock(&buffer_mutex);
    while (item_count == 0) {
        pthread_cond_wait(&cond_empty, &buffer_mutex);
    }

    Item *item = removeItemFromBuffer();
    item_count = item_count - 1;

    if (PRINTF_TYPE == EXPANDED_TYPE) {
        pthread_mutex_lock(&printf_mutex);
        printf("[CONSUMER][%d]: Removing line from buffer: %s", item->idx, item->line);
        pthread_mutex_unlock(&printf_mutex);
    }

    if (item_count == BUFFER_SIZE - 1) {
        pthread_cond_broadcast(&cond_full);
    } else if (item_count == 0 && reading_end == 1) {
        pthread_cond_broadcast(&cond_end);
    }

    pthread_mutex_unlock(&buffer_mutex);
    return item;
}

char *produceLine() {
    pthread_mutex_lock(&read_mutex);
    char *line = malloc(MAX_LINE_LENGTH * sizeof(char));
    if (FILE_IN != NULL) {
        size_t len = (size_t) MAX_LINE_LENGTH;
        if (getline(&line, &len, FILE_IN) == -1) {
            if (NK == 0) {
                printf("[PRODUCER] Reading end.\n");
                reading_end = 1;
            }
            pthread_mutex_unlock(&read_mutex);
            pthread_exit((void *) 0);
        }
        line[strlen(line)] = '\0';
    }

    pthread_mutex_unlock(&read_mutex);
    return line;
}

void *producer(void *param) {
    while (!reading_end) {
        char *line = produceLine();
        add(line);
    }
    pthread_exit((void *) 0);
}

void *consumer(void *param) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    while (1) {
        Item *item = _remove();
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

    while (!(item_count == 0 && reading_end == 1)) {
        pthread_cond_wait(&cond_end, &read_mutex);
    }

    free(producers);
    free(consumers);
}

int main(int argc, char **argv) {
    parse_arguments(argc, argv);
    ARRAY = (char **) malloc(BUFFER_SIZE * sizeof(char *));
    atexit(global_exit_handler);
    threads_start();

    return 0;
}