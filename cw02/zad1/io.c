//
// Created by Dawid Drozd on 15.03.2018.
//

#include <fcntl.h>
#include <assert.h>
#include <zconf.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <stdio.h>
#include <memory.h>
#include <stdbool.h>
#include <time.h>
#include "io.h"

static const char NEW_LINE = '\n';
static const char *random_letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
//abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789

void check_if_correct(bool is_ok, const char *message);

void generat(const char *filename, int block_size, int number_of_records) {
    assert(block_size > 0 && number_of_records > 0);
    srand((unsigned int) time(NULL));

    int descriptor_write = creat(filename, 0644);
    check_if_correct(descriptor_write > 0, filename);

    char *random_data = malloc(block_size * sizeof(char));

    for (int i = 0; i < number_of_records; i++) {
        size_t size_random = strlen(random_letters);
        for (int j = 0; j < block_size; j++) {
            random_data[j] = random_letters[rand() % size_random];
        }
        write(descriptor_write, random_data, (size_t) block_size);
        write(descriptor_write, &NEW_LINE, 1);
    }

    free(random_data);
    close(descriptor_write);
}

void copy_sys(const char *in_file, const char *out_file, int block_size) {
    int descriptor_read = open(in_file, O_RDWR);
    check_if_correct(descriptor_read > 0, in_file);

    int descriptor_write = creat(out_file, S_IRWXG | S_IRWXO | S_IRWXU);
    check_if_correct(descriptor_write > 0, out_file);

    char *buffer = malloc(block_size * sizeof(char));

    ssize_t byte_read;
    ssize_t all_byte_read = 0;
    ssize_t all_byte_written = 0;
    while ((byte_read = read(descriptor_read, buffer, block_size * sizeof(char)))) {
        all_byte_read += byte_read;
        all_byte_written += write(descriptor_write, buffer, (size_t) byte_read);
    }
    assert(all_byte_written == all_byte_read);

    free(buffer);
    close(descriptor_write);
    close(descriptor_read);
}

void copy_lib(const char *in_file, const char *out_file, int block_size) {
    FILE *file_read = fopen(in_file, "r");
    check_if_correct(file_read != NULL, in_file);
    FILE *file_write = fopen(out_file, "wb");
    check_if_correct(file_read != NULL, out_file);

    char *buffer = malloc(block_size * sizeof(char));

    ssize_t byte_read;
    ssize_t all_byte_read = 0;
    ssize_t all_byte_written = 0;
    while ((byte_read = fread(buffer, sizeof(char), (size_t) block_size, file_read))) {
        all_byte_read += byte_read;
        all_byte_written += fwrite(buffer, sizeof(char), (size_t) byte_read, file_write);
    }
    assert(all_byte_written == all_byte_read);

    fclose(file_read);
    fclose(file_write);
    free(buffer);
}

void sort_sys(const char *in_file, int block_size, int number_of_records) {
    int descriptor = open(in_file, O_RDWR);
    check_if_correct(descriptor > 0, in_file);

    char *pom = malloc((size_t) block_size);
    char *j_buff = malloc((size_t) block_size);

    for (int ACTUAL_COMPARE = block_size; ACTUAL_COMPARE >= 0; ACTUAL_COMPARE--) {
        for (int i = 1; i < number_of_records; i++) {
            lseek(descriptor, block_size * i + i, SEEK_SET);
            read(descriptor, pom, (size_t) block_size);

            int j;
            for (j = i - 1; j >= 0; j--) {
                lseek(descriptor, block_size * j + j, SEEK_SET);
                read(descriptor, j_buff, (size_t) block_size);
                if (j_buff[ACTUAL_COMPARE] > pom[ACTUAL_COMPARE]) {
                    lseek(descriptor, block_size * j + j, SEEK_SET);
                    read(descriptor, j_buff, (size_t) block_size);

                    lseek(descriptor, block_size * (j + 1) + (j + 1), SEEK_SET);
                    write(descriptor, j_buff, (size_t) block_size);
                } else
                    break;
            }

            lseek(descriptor, block_size * (j + 1) + (j + 1), SEEK_SET);
            write(descriptor, pom, (size_t) block_size);
        }
    }
    free(pom);
    free(j_buff);
    close(descriptor);
}

void sort_lib(const char *in_file, int block_size, int number_of_records) {
    FILE *file = fopen(in_file, "r+");
    check_if_correct(file != NULL, in_file);

    char *pom = malloc((size_t) block_size);
    char *j_buff = malloc((size_t) block_size);

    for (int ACTUAL_COMPARE = block_size; ACTUAL_COMPARE >= 0; ACTUAL_COMPARE--) {
        for (int i = 1; i < number_of_records; i++) {
            fseek(file, block_size * i + i, SEEK_SET);
            fread(pom, 1, (size_t) block_size, file);

            int j;
            for (j = i - 1; j >= 0; j--) {
                fseek(file, block_size * j + j, SEEK_SET);
                fread(j_buff, sizeof(char), (size_t) block_size, file);
                if (j_buff[ACTUAL_COMPARE] > pom[ACTUAL_COMPARE]) {
                    fseek(file, block_size * j + j, SEEK_SET);
                    fread(j_buff, sizeof(char), (size_t) block_size, file);

                    fseek(file, block_size * (j + 1) + (j + 1), SEEK_SET);
                    fwrite(j_buff, sizeof(char), (size_t) block_size, file);
                } else
                    break;
            }

            fseek(file, block_size * (j + 1) + (j + 1), SEEK_SET);
            fwrite(pom, sizeof(char), (size_t) block_size, file);
        }
    }
    fclose(file);
}

void check_if_correct(bool is_ok, const char *message) {
    if (!is_ok) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), message);
        exit(errno);
    }
}