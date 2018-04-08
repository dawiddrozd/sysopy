#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/time.h>
#include "io.h"

const char *program_name;
const long MILLION = 1000000;

#define MEASURE(print_name, test_block) { \
    print_name; \
    struct timeval real_start, user_start, system_start; \
    struct timeval real_end, user_end, system_end; \
    struct rusage usage; \
    gettimeofday(&real_start, 0); \
    getrusage(RUSAGE_SELF, &usage); \
    user_start = usage.ru_utime; \
    system_start = usage.ru_stime; \
    test_block; \
    gettimeofday(&real_end, 0); \
    getrusage(RUSAGE_SELF, &usage); \
    user_end = usage.ru_utime; \
    system_end = usage.ru_stime; \
    printf("Real: %ldµs\nUser: %ldµs\nSyst: %ldµs\n", \
    (real_end.tv_sec - real_start.tv_sec) * MILLION + real_end.tv_usec - real_start.tv_usec, \
    (user_end.tv_sec - user_start.tv_sec) * MILLION + user_end.tv_usec - user_start.tv_usec, \
    (system_end.tv_sec - system_start.tv_sec) * MILLION + system_end.tv_usec - system_start.tv_usec); \
} \


int parse_string(char *string) {
    char *errptr;
    int number = (int) strtol(string, &errptr, 10);
    if (*errptr != '\0') {
        printf("Can't parse second argument: Only numbers are expected.\n");
        exit(EXIT_FAILURE);
    }
    return number;
}

void print_usage(FILE *stream, int exit_code) {
    fprintf(stream, "Usage: %s [options]\n", program_name);
    fprintf(stream,
            "  -h --help       Print that help info.\n"
                    "  -g --generate <filename> <number_of_records> <block_size>\n"
                    "  -s --sort <filename> <number_of_records> <block_size> <lib/sys>\n"
                    "  -c --copy <in> <out> <number_of_records> <block_size> <lib/sys>\n");
    exit(exit_code);
}

int main(int argc, char *argsv[]) {
    // Nazwa pliku, wielkość oraz ilość rekordów stanowić będą argumenty wywołania programu.

    program_name = argsv[0];
    char *filename = NULL;
    char *output = NULL;
    int block_size = -1;
    int number_of_records = -1;

    bool do_generate = false;
    bool do_sort = false;
    bool do_copy = false;
    bool using_lib = false;
    bool do_test = false;

    static struct option long_options[] = {
            {"generate", required_argument, 0, 'g'},
            {"sort",     required_argument, 0, 's'},
            {"copy",     required_argument, 0, 'c'},
            {"help",     required_argument, 0, 'h'},
            {"test",     no_argument,       0, 't'},
            {0, 0,                          0, 0}
    };

    int opt = 0;
    int long_index = 0;
    int index;

    opt = getopt_long(argc, argsv, "hg:s:c:", long_options, &long_index);
    switch (opt) {
        case 'g':
            if (argc != 5) {
                print_usage(stdout, EXIT_FAILURE);
            }
            do_generate = true;
            index = optind - 1;
            filename = argsv[index];
            number_of_records = parse_string(argsv[++index]);
            block_size = parse_string(argsv[++index]);
            break;
        case 's':
            if (argc != 6) {
                print_usage(stdout, EXIT_FAILURE);
            }
            do_sort = true;
            index = optind - 1;
            filename = argsv[index];
            number_of_records = parse_string(argsv[++index]);
            block_size = parse_string(argsv[++index]);
            using_lib = (bool) strcmp(argsv[++index], "lib");
            break;
        case 'c':
            if (argc != 7) {
                print_usage(stdout, EXIT_FAILURE);
            }
            do_copy = true;
            index = optind - 1;
            filename = argsv[index];
            output = argsv[++index];
            number_of_records = parse_string(argsv[++index]);
            block_size = parse_string(argsv[++index]);
            using_lib = (bool) strcmp(argsv[++index], "lib");
            break;
        case 't':
            do_test = true;
            break;
        case 'h':
            print_usage(stdout, 0);
        default:
            print_usage(stdout, EXIT_FAILURE);
    }

    if (do_generate) {
        generat(filename, block_size, number_of_records);
    } else if (using_lib) {
        if (do_copy) copy_lib(filename, output, block_size);
        if (do_sort) sort_lib(filename, block_size, number_of_records);
    } else {
        if (do_copy) copy_sys(filename, output, block_size);
        if (do_sort) sort_sys(filename, block_size, number_of_records);
    }

    if (do_test) {
        int tests[] = {4, 512, 4096, 8192};

        for (int i = 0; i < 4; i++) {
            char test_filename[] = "generatedA.txt";
            char sort_lib_filename[] = "sorted_libA.txt";
            char sort_sys_filename[] = "sorted_sysA.txt";
            test_filename[9] = (char) (i + '0');
            sort_lib_filename[10] = (char) (i + '0');
            sort_sys_filename[10] = (char) (i + '0');

            int test_number_of_records = 16;
            generat(test_filename, tests[i], test_number_of_records);

            printf("#########################################\n");
            printf("BLOCK SIZE = %d, NUMBER OF RECORD = %d\n", tests[i], test_number_of_records);
            printf("#########################################\n");

            MEASURE(printf("Copying SYSTEM:\n"),
                    copy_sys(test_filename, sort_sys_filename, tests[i]));
            MEASURE(printf("\nCopying LIBRARY\n"),
                    copy_lib(test_filename, sort_lib_filename, tests[i]));
            MEASURE(printf("\nSorting SYSTEM:\n"),
                    sort_sys(sort_sys_filename, tests[i], test_number_of_records));
            MEASURE(printf("\nSorting LIBRARY:\n"),
                    sort_lib(sort_lib_filename, tests[i], test_number_of_records));
            printf("\n");
        }
    }

    return 0;
}