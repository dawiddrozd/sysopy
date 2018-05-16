#include <pthread.h>
#include <stdio.h>
#include <memory.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#define BUFFER_SIZE 128

typedef struct Array {
    int x_size;
    int y_size;
    int **array;
} Array;

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

void parse(Array arr, FILE *stream) {
    char *line = NULL;
    char *num;
    ssize_t nread;
    int y_actual = 0;
    int x_actual = 0;
    int nums = 0;

    while ((nread = get_next_line(stream, &line) != -1)) {
        num = strtok(line, " \t");
        do {
            arr.array[x_actual][y_actual++] = to_int(num);
            nums++;
            if (y_actual >= arr.y_size) {
                y_actual = 0;
                x_actual++;
            }
        } while ((num = strtok(NULL, " \n\t")) != NULL);
    }
    check_exit(nums != arr.y_size * arr.x_size, "Not proper numer of numbers");
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

    parse(pgm, stream);

    free(line);
    fclose(stream);
    return pgm;
}

Array parse_filter(const char *in) {
    FILE *stream = NULL;
    char *line = NULL;
    char *num;
    Array filter;
    ssize_t nread;

    stream = fopen(in, "rw");
    check_exit(stream == NULL, "Can't open in file");

    nread = get_next_line(stream, &line);
    num = strtok(line, " \n\t");
    int size = to_int(num);

    filter.y_size = size;
    filter.x_size = size;
    filter.array = (int **) malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++) {
        filter.array[i] = (int *) malloc(size * sizeof(int));
    }

    parse(filter, stream);
    return filter;
}

int main(int argc, char **argv) {
    check_exit(argc < 4, "Incorrect number of arguments. Example: main.out 4 in.pgm filter.txt out.pgm");

    int nr_threads = to_int(argv[1]);
    char *in = argv[2];
    char *fltr = argv[3];
    char *out = argv[4];
    Array pgm;
    Array filter;

    pgm = parse_pgm_file(in);
    //print(pgm);
    filter = parse_filter(fltr);
    //print(filter);


    return 0;
}