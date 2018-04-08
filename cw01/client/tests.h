//
// Created by Dawid Drozd on 09.03.2018.
//

#ifndef SYSOPY1_TESTS_H
#define SYSOPY1_TESTS_H

Block *test_create_block(int num_elements, int block_size, int);
char *test_search(Block *block);
void test_del_add(Block *block, int size);
void test_del_add_alternally(Block *block, int size);

#endif //SYSOPY1_TESTS_H