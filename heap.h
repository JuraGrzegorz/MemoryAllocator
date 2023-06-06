#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>


enum pointer_type_t
{
    pointer_null,
    pointer_heap_corrupted,
    pointer_control_block,
    pointer_inside_fences,
    pointer_inside_data_block,
    pointer_unallocated,
    pointer_valid
};

struct memory_manager_t
{
    void *memory_start;
    int flag;
    struct memory_chunk_t *first_memory_chunk;
};

struct memory_chunk_t
{
    struct memory_chunk_t* prev;
    struct memory_chunk_t* next;
    size_t size;
    int free;
    long long flag;
};
struct memory_manager_t memory_manager;

int heap_setup(void);
void heap_clean(void);
void* heap_malloc(size_t size);
void* heap_calloc(size_t number, size_t size);
void* heap_realloc(void* memblock, size_t count);
void  heap_free(void* memblock);
int heap_validate(void);
enum pointer_type_t get_pointer_type(const void* const pointer);
size_t   heap_get_largest_used_block_size(void);
void flag_create(struct memory_chunk_t **val,long long *flag);
