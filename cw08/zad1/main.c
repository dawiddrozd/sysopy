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

#define BUFFER_SIZE 128
#define INT_PARSE 0
#define DOUBLE_PARSE 1

typedef struct Array {
    int x_size;
    int y_size;
    int filter_size;
    int **array;
    double **filter;
} Array;

void free_all(Array array);

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
            printf("%d ", array.array[i][j]);
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
                arr->array[x_actual][y_actual++] = to_int(num);
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

Array parse_pgm_file(const char *in) {
    FILE *stream = NULL;
    char *line = NULL;
    char *num;
    ssize_t nread;
    Array pgm;

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

    pgm.x_size = x_size;
    pgm.y_size = y_size;
    pgm.array = (int **) malloc(x_size * sizeof(int *));
    for (int i = 0; i < x_size; i++) {
        pgm.array[i] = (int *) malloc(y_size * sizeof(int));
    }

    parse(&pgm, stream, INT_PARSE);

    free(line);
    fclose(stream);
    return pgm;
}

void parse_filter(const char *in, Array *filter) {
    FILE *stream = NULL;
    char *line = NULL;
    char *num;
    ssize_t nread;

    stream = fopen(in, "rw");
    check_exit(stream == NULL, "Can't open in file");

    nread = get_next_line(stream, &line);
    num = strtok(line, " \n\t");
    int size = to_int(num);
    filter->filter_size = size;
    filter->filter = (double **) malloc(size * sizeof(double *));
    for (int i = 0; i < size; i++) {
        filter->filter[i] = (double *) malloc(size * sizeof(double));
    }
    parse(filter, stream, DOUBLE_PARSE);
}

int main(int argc, char **argv) {
    check_exit(argc < 4, "Incorrect number of arguments. Example: main.out 4 in.pgm filter.txt out.pgm");

    int nr_threads = to_int(argv[1]);
    char *in = argv[2];
    char *fltr = argv[3];
    char *out = argv[4];
    Array pgm;

    pgm = parse_pgm_file(in);
    parse_filter(fltr, &pgm);
    //print(pgm);

    free_all(pgm);
    return 0;
}

void free_all(Array array) {
    for(int i =0; i<array.x_size; i++) {
        free(array.array[i]);
    }
    free(array.array);

    for(int i=0; i<array.filter_size; i++) {
        free(array.filter[i]);
    }
    free(array.filter);
}