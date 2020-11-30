#ifndef __BUDDY_H_
#define __BUDDY_H_

#define E_NO_SPACE            1
#define E_CORRUPT_FREESPACE   2
#define E_PADDING_OVERWRITTEN 3
#define E_BAD_ARGS            4
#define E_BAD_POINTER         5


#define BITS_PER_WORD 32
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)
#define MAX_SIZE 1048576
#define MIN_BUDDY_SIZE 1024
#define MAX_ORDER (int)(log2(MAX_SIZE / MIN_BUDDY_SIZE))
#define BITMAP_BITS (MAX_SIZE / MIN_BUDDY_SIZE * 2)
#define BITMAP_SIZE (BITMAP_BITS >> 3)
#define BITMAP_WORD_COUNT (BITMAP_BITS / BITS_PER_WORD)


extern int m_error;
extern void* base;
extern uint32_t is_blocked[BITMAP_WORD_COUNT];
extern uint32_t is_allocated[BITMAP_WORD_COUNT];

int mem_init(size_t size_of_region);
void* mem_alloc(size_t size);
int mem_free(void* ptr);
void mem_dump();

int get(uint32_t bitmap[], int i);
void set(uint32_t bitmap[], int i);
void unset(uint32_t bitmap[], int i);
void clear_bitmap(uint32_t bitmap[], size_t size);
void fill_bitmap(uint32_t bitmap[], size_t size);
void init_blocked_bitmap();
void init_allocated_bitmap();
void validate_available();
void dump_bits(uint32_t word);
void dump_bitmap(uint32_t bitmap[], size_t word_count);
size_t size2order(size_t size);
size_t order2entry(size_t order);
size_t index2order(size_t i);
void* index2ptr(size_t i);
size_t get_buddy(size_t index);
size_t get_parent(size_t index);
int free_block(size_t index);
int ptr2index(void* ptr);
ssize_t search(size_t order);

#endif