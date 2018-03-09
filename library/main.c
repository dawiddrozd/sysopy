#include <stdio.h>
#include <assert.h>
#include "lib.h"
int main() {

    int size = 5;
    Block* block = create(size);

    add(block,"i1");
    add(block,"i2");
    add(block,"i5");
    add(block,"i6");
    delete_char(block,"i1");
    add(block,"i3");
    delete_char(block,"i4");
    delete_char(block,"i5");
    add(block, "i7");
    add(block, "i11");
    add(block, "i12");

    //print(block);
    search_for(block);

    return 0;
}