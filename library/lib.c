#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "lib.h"

void print_parameter(const char *parameter);

char **realloc_with_null(Block *block);

int sum_for_string(const char *text, int idx);

Block *create(int num_elements, int block_size) {
    assert(num_elements > 0);

    Block *block = calloc(1, sizeof(Block));

    if (block == NULL)
        return NULL;

    block->array = calloc((size_t) num_elements, sizeof(char *)); // do not add zero

    for (int i = 0; i < num_elements; i++) {
        block->array[i] = calloc((size_t) block_size, sizeof(char));
        if (block->array[i] == NULL) {
            printf("Out of memory!\n");
            exit(EXIT_FAILURE);
        }
    }
    block->size = num_elements;
    block->used = 0;
    block->block_size = block_size;

    if (block->array == NULL) {
        printf("Out of memory!\n");
        exit(EXIT_FAILURE);
    }

    return block;
}

void add(Block *block, char *text) {
    if (block->used < block->size) {
        int free = block->used;
        int loop = -1;
        while (*block->array[free] != 0) {
            free++;
            free = free % (block->size);
            loop++;
            assert(loop != block->size);
        }
        //size_t text_len = strlen(text);
        //block->array[free] = calloc(128, sizeof(char));
        if ((strlen(text) + 1) > block->block_size) {
            printf("Can't allocate memory! Block size is too small.\n");
            return;
        }
        strcpy(block->array[free], text);
        block->used++;

    } else {
        block->array = realloc_with_null(block);
        add(block, text);
    }
}

char **realloc_with_null(Block *block) {
    assert(block->size == block->used);

    char **tmp = (char **) realloc(block->array, 2 * block->size * sizeof *block->array);

    if (tmp) {
        block->array = tmp;
        for (size_t i = 0; i < block->size; i++) {
            block->array[block->size + i] = calloc((size_t) block->block_size, sizeof(char));
            if (block->array[block->size + i] == NULL) {
                printf("Out of memory!\n");
                exit(EXIT_FAILURE);
            }
        }
        block->size *= 2;
    }

    //print(block);
    return block->array;
}

void print(Block *block) {
    int used = 0;
    int nulls = 0;
    for (int i = 0; i < block->size; i++) {
        *block->array[i] == 0 ? nulls++ : used++;
        printf("%3d. %s\n", i + 1, block->array[i]);

    }
    printf("Nulls: %d\nUsed: %d\nReal: %d\n", nulls, used, block->used);
}

void delete_char(Block *block, const char *to_delete) {
    for (int i = 0; i < block->size; i++) {
        if (block->array[i] != NULL && strcmp(to_delete, block->array[i]) == 0) {
            /* or block->array[i] == to_delete */
            free(block->array[i]);
            block->array[i] = calloc((size_t) block->block_size, sizeof(char));
            block->used--;
        }
    }
}

void delete_all(Block *block) {
    for (int i = 0; i < block->size; i++) {
        free(block->array[i]);
        block->array[i] = NULL;
    }
    free(block->array);
    free(block);
    block = NULL;
}

void delete_some(Block *block, int num_to_delete) {
    if (num_to_delete > block->used) {
        delete_all(block);
        return;
    }
    int idx;
    srand((unsigned int) time(NULL));
    int deleted = 0;
    while (deleted != num_to_delete) {
        idx = rand() % block->size;
        if (*block->array[idx] != 0) {
            free(block->array[idx]);
            block->array[idx] = calloc((size_t) block->block_size, sizeof(char));
            block->used--;
            deleted++;
        }
    }
}

char *search_for(Block *block) {
    if (block->used <= 0) {
        return NULL;
    }
    char *best;
    int best_idx;
    int best_diff;
    best = block->array[0];
    best_idx = 0;
    best_diff = sum_for_string(best, 0);

    for (int i = 0; i < block->size; i++) {
        if (block->array[i] != 0) {
            int tmp_sum = sum_for_string(block->array[0], i);
            if (tmp_sum < best_diff) {
                best_diff = tmp_sum;
                best_idx = i;
                best = block->array[i];
            }
        }
    }
    printf("Best difference is for:\n"
                   "%s and it is %d\n", best, best_diff);
    return best;
}

int sum_for_string(const char *text, int idx) {
    const char *p;
    int sum = 0;
    for (p = text; *p != '\0'; p++) {
        sum += *p;
    }
    return abs(sum - idx);
}
