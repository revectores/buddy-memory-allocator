#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "buddy.h"



void test_size2order(){
    assert(size2order(MIN_BUDDY_SIZE) == 0);
    assert(size2order(MIN_BUDDY_SIZE + 1) == 1);
    assert(size2order(MAX_SIZE) == 10);
    assert(size2order(MAX_SIZE + 1) == 11);
}

void test_order2entry(){
    assert(order2entry(0) == 1023);
    assert(order2entry(10) == 0);
}

void test_index2order(){
    assert(index2order(0) == 10);
    assert(index2order(1) == 9);
    assert(index2order(2) == 9);
    assert(index2order(1022) == 1);
    assert(index2order(1023) == 0);
    assert(index2order(1024) == 0);
}

void test_index2ptr(){
    assert(index2ptr(1025) == base + 2 * 1024);
}

void test_get_buddy(){
    assert(get_buddy(1) == 2);
    assert(get_buddy(2) == 1);
}


void test_equal_size_alloc(){
    validate_available();

    void* ptrs[10];

    for(int i = 0; i < 4; i++){
        ptrs[i] = mem_alloc(MAX_SIZE / 8);
        dump_bitmap(is_blocked, 1);
        dump_bitmap(is_allocated, 1);
        printf("ptrs[%d] - base = %ld\n", i, ptrs[i] - base);
    }

    for(int i = 3; i >= 0; i--){
        mem_free(ptrs[i]);
        dump_bitmap(is_blocked, 1);
        dump_bitmap(is_allocated, 1);
    }
    validate_available();
}


void test_random_free(){
    clock_t start = clock();

    mem_init(MAX_SIZE);
    // mem_dump();
    void* ptr[10];
    ptr[0] = mem_alloc(128);
    ptr[1] = mem_alloc(128);
    ptr[2] = mem_alloc(128);
    ptr[3] = mem_alloc(128);
    ptr[4] = mem_alloc(128);
    ptr[5] = mem_alloc(128);
    ptr[6] = mem_alloc(128);

    for (int i = 0; i < 7; i++){
        printf("ptr[%d] = %p\n", i, ptr[i]);
    }

    mem_free(ptr[4]);
    mem_free(ptr[1]);
    mem_free(ptr[2]);
    mem_free(ptr[6]);
    
    // mem_alloc(100);

    clock_t diff = clock() - start;
    printf("CLOCKS_PER_SEC = %d, clock_diff = %lu\n", CLOCKS_PER_SEC, diff);
    int msec = diff * 1000 / CLOCKS_PER_SEC;

    printf("time cost: %d ms", msec);
}


int main(){
    test_random_free();
    return 0;
}
