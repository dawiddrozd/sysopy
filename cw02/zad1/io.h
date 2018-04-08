//
// Created by Dawid Drozd on 15.03.2018.
//

#ifndef SYSOPY2_IO_H
#define SYSOPY2_IO_H

void generat(const char *filename, int block_size, int number_of_records);
void copy_sys(const char *in_file, const char *out_file, int block_size);
void copy_lib(const char *in_file, const char *out_file, int block_sizes);
void print_blocks(const char *in_file, int block_size);
void sort_sys(const char *in_file, int block_size, int number_of_records);
void sort_lib(const char *in_file, int block_size, int number_of_records);

#endif //SYSOPY2_IO_H