#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "blocks.h"

char *global_array[200000];

char **realloc_with_zeros(Block *block);

int sum_for_string(const char *text, int idx);

Block *create(int num_elements, int block_size, int is_static) {
    assert(num_elements > 0);

    Block *block = calloc(1, sizeof(Block));

    if (block == NULL)
        return NULL;

    if (is_static) {
        block->array = global_array;
    } else {
        block->array = calloc((size_t) num_elements, sizeof(char *)); // do not add zero
    }

    for (int i = 0; i < num_elements; i++) {
        block->array[i] = calloc((size_t) block_size, sizeof(char));
        if (block->array[i] == NULL) {
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
        }
    }

    block->size = num_elements;
    block->used = 0;
    block->block_size = block_size;
    block->is_static = is_static;

    if (block->array == NULL) {
        fprintf(stderr, "Out of memory!\n");
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
            assert (loop != block->size);
        }

        if ((strlen(text) + 1) > block->block_size) {
            fprintf(stderr, "Can't allocate memory! Block size is too small.\n");
            return;
        }
        strcpy(block->array[free], text);
        block->used++;

    } else {
        fprintf(stderr, "Can't allocate memory! Block size is too small.\n");
    }
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
    if (!block->is_static) {
        free(block->array);
    }
    free(block);
    block = NULL;
}

void delete_some(Block *block, int num_to_delete) {
    if (num_to_delete > block->used) {
        delete_all(block);
        return;
    }

    int idx;
    idx = rand() % block->size;
    srand((unsigned int) time(NULL));
    int deleted = 0;
    while (deleted != num_to_delete) {
        for (; deleted < num_to_delete;) {
            idx++;
            idx = idx % block->size;
            if (*block->array[idx] != 0) {
                free(block->array[idx]);
                block->array[idx] = calloc((size_t) block->block_size, sizeof(char));
                block->used--;
                deleted++;
            }
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
    //printf("Best difference is for:\n%s and it is %d\n", best, best_diff);
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