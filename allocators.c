#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include "database_internal.h"

static void* scratch_mem;
static u64 scratch_off;

void db_scratch_init()
{
    scratch_off = 0;
    scratch_mem = calloc(MAX_SCRATCH_SIZE, 1);
    if (scratch_mem == NULL)
    {
        printf("NULL SCRATCH!!\n");
    }
}

void db_scratch_uninit()
{
    scratch_off = 0;
    free(scratch_mem);
}

void* db_scratch_alloc(u64 size)
{
    assert(size < MAX_SCRATCH_SIZE);

    if (scratch_off + size > MAX_SCRATCH_SIZE)
    {
        // go back to beginning
        scratch_off = 0;
    }
    // if we still have room
    else
    {
        scratch_off += size;
    }

    return scratch_mem + (scratch_off - size);
}

// just a bump allocator for now
static void* arena_mem;
static u64   arena_size;
static u64   arena_capacity;

void db_arena_init(void)
{
    arena_capacity  = ARENA_BLOCK_SIZE;
    arena_mem       = calloc(arena_capacity, 1);
    arena_size      = 0;
}
void db_arena_uninit(void)
{
    arena_capacity  = 0;
    arena_size      = 0;
    free(arena_mem);
}
// returns NULL if size exceeds max scratch size
void* db_arena_alloc(u64 size)
{
    if (arena_size + size > arena_capacity)
    {
        void* new_ptr;
        arena_capacity += arena_size * 2 + size;
        new_ptr = realloc(arena_mem, arena_capacity);

        if (new_ptr == NULL)
        {
            printf("We have a massive problem here\n");
            assert(0);
        }
        else
        {
            arena_mem = new_ptr;
        }
    }
    arena_size += size;
    return arena_mem + (arena_size - size);
}
void* db_arena_calloc(u64 size, u64 count)
{
    void* ptr = db_arena_alloc(size * count);

    if (ptr != NULL)
    {
        bzero(ptr, size * count);
    }

    return ptr;
}
void* db_arena_realloc(void* ptr, u64 prev_size, u64 size)
{
    if (size < prev_size)
        return ptr;
    else
    {
        void* new_ptr = db_arena_alloc(size);

        // check ptr is in range
        if (ptr >= arena_mem && ptr < arena_mem + arena_capacity)
        {
            memcpy(new_ptr, ptr, prev_size);

            return new_ptr;
        }
        else
        {
            return new_ptr;   
        }
    }
}



