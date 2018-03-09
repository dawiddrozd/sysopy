//
// Created by Dawid Drozd on 27.02.2018.
//
#ifndef SYSOPY1_DOUBLELINKEDLIST_H
#define SYSOPY1_DOUBLELINKEDLIST_H

typedef struct Block {
    char** array;
    int size;
    int used;
    int block_size;
} Block;

#ifdef DYNAMIC
    void *lib;
#endif
Block* create(int,int);
void add(Block*,char*);
void print(Block*);
void delete_char(Block *, const char *);
void delete_all(Block*);
char *search_for(Block *);
void delete_some(Block *, int);

#endif
