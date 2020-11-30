#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "buddy.h"


int m_error;
void* base = NULL;
uint32_t is_blocked[BITMAP_WORD_COUNT];
uint32_t is_allocated[BITMAP_WORD_COUNT];


int get(uint32_t bitmap[], int i){
    uint32_t bit = bitmap[WORD_OFFSET(i)] & (1 << BIT_OFFSET(i));
    return bit; 
}


void set(uint32_t bitmap[], int i){
    bitmap[WORD_OFFSET(i)] |= (1 << BIT_OFFSET(i));
}


void unset(uint32_t bitmap[], int i){
    bitmap[WORD_OFFSET(i)] &= ~(1 << BIT_OFFSET(i));
}


void clear_bitmap(uint32_t bitmap[], size_t size){
    memset(bitmap, 0, size);
}


void fill_bitmap(uint32_t bitmap[], size_t size){
    memset(is_blocked, 0xff, size);
}


void init_blocked_bitmap(){
    fill_bitmap(is_blocked, BITMAP_SIZE);
    unset(is_blocked, 0);
}


void init_allocated_bitmap(){
    clear_bitmap(is_allocated, BITMAP_SIZE);
}


/* Our implementation of buddy system requires that both the children must be
   set to 1 (unavaliable) if the parent is set to 0 (avaliable) */
void validate_available(){
    for (int i = 0; i < BITMAP_BITS / 2; i++){
        assert(get(is_blocked, i) || ((get(is_blocked, i * 2 + 1) && get(is_blocked, i * 2 + 2))));
    }
}


void dump_bits(uint32_t word){
    for (int i = 0; i < sizeof(word) * 8; i++){
        printf("%u", (word >> i) & 1);
        if ((i + 1) % 8 == 0) printf(" ");
    }
}


void dump_bitmap(uint32_t bitmap[], size_t word_count){
    for (int i = 0; i < word_count; i++){
        printf("%2d:    ", i);
        dump_bits(bitmap[i]);
        printf("\n");
    }
}


int mem_init(size_t size_of_region){
    init_blocked_bitmap();
    init_allocated_bitmap();
    base = mmap(NULL, size_of_region, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
    return 0;
}


size_t size2order(size_t size){
    int order = (int)log2(size - 1) - 9;
    if (order < 0) order = 0;
    return order;
}


size_t order2entry(size_t order){
    size_t entry = 1;
    entry <<= (MAX_ORDER - order);
    entry -= 1;
    return entry;
}


size_t index2order(size_t i){
    return MAX_ORDER - (int)log2(i+1);
}


void* index2ptr(size_t i){
    assert(base);
    size_t order = index2order(i);
    size_t entry = order2entry(order);
    size_t offset = i - entry;
    return base + (offset << (order + 10));
}


size_t get_buddy(size_t index){
    return ((index + 1) ^ 1) - 1;
}


size_t get_parent(size_t index){
    return (index - 1) / 2;
}


int free_block(size_t index){
    // if (!get(is_blocked, index) || !get(is_allocated, index)){
    //     return -1;
    // }
    
    unset(is_blocked, index);
    unset(is_allocated, index);

    if (index == 0){
        return 0;
    }

    size_t buddy_index = get_buddy(index);
    if (!get(is_allocated, buddy_index) && !get(is_blocked, buddy_index)) {
        // printf("buddy free found: %zu\n", buddy_index);
        set(is_blocked, index);
        set(is_blocked, buddy_index);
        free_block(get_parent(index));
    }
    return 0;
}


int ptr2index(void* ptr){
    assert(base);
    size_t offset = (ptr - base) / MIN_BUDDY_SIZE;
    int order = 0;
    while (offset % 2 == 0 && order <= MAX_ORDER) {
        int index = order2entry(order) + offset;
        printf("offset = %5zu, order = %5d, index = %5d\n", offset, order, index);
        if (get(is_allocated, index)) {
            return index;
        }
        order ++;
        offset /= 2;
    }
    int index = order2entry(order) + offset;
    printf("offset = %5zu, order = %5d, index = %5d\n", offset, order, index);
    if (get(is_allocated, index)) {
        return index;
    }
    return -1;
}


ssize_t search(size_t order){
    size_t entry = order2entry(order);
    size_t next_entry = order2entry(order - 1);
    for (size_t i = entry; i < next_entry; i++){
        if (get(is_blocked, i) == 0) {
            set(is_allocated, i);
            set(is_blocked, i);
            return i;
        }
    }
    if (order < MAX_ORDER) {
        size_t upper_free_i = search(order + 1);
        if (upper_free_i < 0) return -1;

        size_t free_i       = upper_free_i * 2 + 1;
        size_t buddy_free_i = free_i + 1;
        unset(is_allocated, upper_free_i);
        set(is_blocked, free_i);
        set(is_allocated, free_i);
        unset(is_blocked, buddy_free_i);
        return free_i;
    }

    return -1;
}


void* mem_alloc(size_t size){
    size_t order = size2order(size);
    if (order > MAX_ORDER) {
        m_error = E_NO_SPACE;
        return NULL;
    }

    void* free_ptr = index2ptr(search(order));

    if (!free_ptr) {
        m_error = E_NO_SPACE;
        return NULL;
    }
    return (void*) free_ptr;
}



int mem_free(void * ptr){
    ssize_t index = ptr2index(ptr);
    free_block(index);
    return 0;
}


void mem_dump(){
    printf("====================DUMP====================\n");
}

