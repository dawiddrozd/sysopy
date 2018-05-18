#include <pthread.h>
#include <stdio.h>
#include <memory.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <sys/time.h>

#define BUFFER_SIZE 128
#define INT_PARSE 0
#define DOUBLE_PARSE 1
#define NUM_PER_LINE 17
#define MILLION 1000000

#define MEASURE(print_name, test_block) { \
    print_name; \
    struct timeval real_start, real_end; \
    struct rusage usage; \
    gettimeofday(&real_start, 0); \
    getrusage(RUSAGE_SELF, &usage); \
    test_block; \
    gettimeofday(&real_end, 0); \
    getrusage(RUSAGE_SELF, &usage); \
    printf("Real execution time: %ldµs\n", \
    (real_end.tv_sec - real_start.tv_sec) * MILLION + real_end.tv_usec - real_start.tv_usec); \
}

typedef struct Array {
    int x_size;
    int y_size;
    int filter_size;
    int max_grey_value;
    int **pgm;
    double **filter;
    int **out;
} Array;

typedef struct Data {
    Array *array;
    int first_x;
    int last_x;
} Data;

void free_all(Array array) {
    for (int i = 0; i < array.x_size; i++) {
        free(array.pgm[i]);
        free(array.out[i]);
    }
    free(array.pgm);
    free(array.out);

    for (int i = 0; i < array.filter_size; i++) {
        free(array.filter[i]);
    }
    free(array.filter);
}

void check_exit(bool correct, const char *message) {
    if (correct) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), message);
        exit(errno);
    }
}

int to_int(char *string) {
    if (string == NULL) {
        fprintf(stderr, "Can't parse number.\n");
        return -1;
    }

    char *errptr;
    int number = (int) strtol(string, &errptr, 10);
    if (*errptr != '\0') {
        fprintf(stderr, "Can't parse number.\n");
        return -1;
    }
    return number;
}

void print(Array array) {
    for (int i = 0; i < array.x_size; i++) {
        for (int j = 0; j < array.y_size; j++) {
            printf("%d ", array.pgm[i][j]);
        }
        printf("\n");
    }
    for (int i = 0; i < array.filter_size; i++) {
        for (int j = 0; j < array.filter_size; j++) {
            printf("%f ", array.filter[i][j]);
        }
        printf("\n");
    }
}

bool is_commented(const char *line) {
    if (line == NULL)
        return false;
    return line[0] == '#';
}

ssize_t get_next_line(FILE *stream, char **line) {
    size_t len = 0;
    ssize_t nread;
    do {
        nread = getline(line, &len, stream);
    } while (is_commented(*line) && nread != -1);
    return nread;
}

void parse(Array *arr, FILE *stream, int type) {
    char *line = NULL;
    char *num;
    ssize_t nread;
    int y_actual = 0;
    int x_actual = 0;
    int nums = 0;

    while ((nread = get_next_line(stream, &line) != -1)) {
        num = strtok(line, " \t");
        do {
            if (type == INT_PARSE) {
                arr->pgm[x_actual][y_actual++] = to_int(num);
            } else {
                arr->filter[x_actual][y_actual++] = strtod(num, NULL);
            }
            nums++;
            if ((y_actual >= arr->y_size && type == INT_PARSE) ||
                (y_actual >= arr->filter_size && type == DOUBLE_PARSE)) {
                y_actual = 0;
                x_actual++;
            }
        } while ((num = strtok(NULL, " \n\t")) != NULL);
    }
    bool checked = true;
    if (type == INT_PARSE) {
        checked = nums != arr->y_size * arr->x_size;
    } else {
        checked = nums != arr->filter_size * arr->filter_size;
    }
    check_exit(checked, "Not proper number of numbers");
}

void parse_pgm_file(Array *pgm, const char *in) {
    FILE *stream = NULL;
    char *line = NULL;
    char *num;
    ssize_t nread;

    stream = fopen(in, "rw");
    check_exit(stream == NULL, "Can't open in file");

    nread = get_next_line(stream, &line);
    check_exit(nread < 2 || line[0] != 'P' || line[1] != '2', "Incorrect file format. P2 expected.");

    nread = get_next_line(stream, &line);
    num = strtok(line, " \t");
    int x_size = to_int(num);
    num = strtok(NULL, " \t\n");
    int y_size = to_int(num);

    nread = get_next_line(stream, &line);
    num = strtok(line, " \n\t");
    int max_grey_value = to_int(num);

    pgm->max_grey_value = max_grey_value;
    pgm->x_size = x_size;
    pgm->y_size = y_size;
    pgm->pgm = (int **) malloc(x_size * sizeof(int *));
    pgm->out = (int **) malloc(x_size * sizeof(int *));
    for (int i = 0; i < x_size; i++) {
        pgm->pgm[i] = (int *) malloc(y_size * sizeof(int));
        pgm->out[i] = (int *) malloc(y_size * sizeof(int));
    }

    parse(pgm, stream, INT_PARSE);

    free(line);
    fclose(stream);
}

void parse_filter(const char *in, Array *filter) {
    FILE *stream = NULL;
    char *line = NULL;
    char *num;
    ssize_t nread;

    stream = fopen(in, "rw");
    check_exit(stream == NULL, "Can't open in file");

    nread = get_next_line(stream, &line);
    check_exit(nread == -1, "Incorrect filter format.");
    num = strtok(line, " \n\t");
    int size = to_int(num);
    check_exit(nread == -1, "Incorrect filter format.");
    filter->filter_size = size;
    filter->filter = (double **) malloc(size * sizeof(double *));
    for (int i = 0; i < size; i++) {
        filter->filter[i] = (double *) malloc(size * sizeof(double));
    }
    parse(filter, stream, DOUBLE_PARSE);
}

int max(int one, int two) {
    return one > two ? one : two;
}

int generate_sxy(Array *arr, int x, int y) {
    double new_val = 0;
    int y_idx;
    int x_idx;
    int c = arr->filter_size;
    for (int i = 0; i < c; i++)
        for (int j = 0; j < c; j++) {
            y_idx = max(1, (x + 1) - (int) ceil((double) c / 2.0) + (i + 1)) - 1;
            x_idx = max(1, (y + 1) - (int) ceil((double) c / 2.0) + (j + 1)) - 1;
            new_val += arr->pgm[x_idx][y_idx] * arr->filter[i][j];
        }
    new_val = round(new_val);
    return (int) new_val % arr->max_grey_value;
}

void generate(Array *arr) {
    for (int i = 0; i < arr->x_size; i++) {
        for (int j = 0; j < arr->y_size; j++) {
            arr->out[i][j] = 0; /*generate_sxy(arr, i, j)*/;
        }
    }
}

void *run(void *param) {
    Data *data = (Data *) param;
    for (int i = data->first_x; i < data->last_x; i++) {
        for (int j = 0; j < data->array->y_size; j++) {
            data->array->out[i][j] = generate_sxy(data->array, i, j);
        }
    }
    return 0;
}

void save_as(const char *out, Array *arr) {
    char buffer[BUFFER_SIZE];
    FILE *stream = fopen(out, "wb");
    check_exit(stream == NULL, "Can't create file");
    sprintf(buffer, "P2\n%d %d\n256\n", arr->x_size, arr->y_size);
    fwrite(buffer, sizeof(char), strlen(buffer) * sizeof(char), stream);

    int counter = 0;
    for (int i = 0; i < arr->x_size; i++) {
        for (int j = 0; j < arr->y_size; j++) {
            sprintf(buffer, "%d ", arr->out[i][j]);
            fwrite(buffer, sizeof(char), strlen(buffer), stream);
            if (++counter > NUM_PER_LINE) {
                fwrite("\n", sizeof(char), 1, stream);
                counter = 0;
            }
        }
    }

    fclose(stream);
}

void threads_start(Array pgm, int nr_threads) {
    pthread_t *threads = malloc(nr_threads * sizeof(pthread_t));
    Data *datas = malloc(nr_threads * sizeof(Data));
    int one_fragment;
    for (int i = 0; i < nr_threads; i++) {
        datas[i].array = &pgm;
        one_fragment = datas[i].array->x_size / nr_threads;
        datas[i].first_x = one_fragment * i;
        datas[i].last_x = one_fragment * i + one_fragment;
        pthread_create(&threads[i], NULL, run, &datas[i]);
    }
    void *returned;
    for (int i = 0; i < nr_threads; i++) {
        pthread_join(threads[i], &returned);
    }
    free(threads);
    free(datas);
}

int main(int argc, char **argv) {
    check_exit(argc < 4, "Incorrect number of arguments. "
            "Example: main.out 4 in.pgm filter.txt out.pgm");

    int nr_threads = to_int(argv[1]);
    char *in = argv[2];
    char *fltr = argv[3];
    char *out = argv[4];
    Array pgm;
    parse_pgm_file(&pgm, in);
    parse_filter(fltr, &pgm);

    MEASURE(printf("Execution using %d numbers of threads\n", nr_threads), threads_start(pgm, nr_threads));

    save_as(out, &pgm);
    free_all(pgm);
    return 0;
}