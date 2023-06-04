#ifndef DATABASE_INTERNAL_H
#define DATABASE_INTERNAL_H

#include <stdio.h>
#include "database.h"

/* Database g_db; */
/* void* g_db_memory; */
/* void* g_db_scratch; */

/* void* db_arena_init(u64 max_size); */
/* void* db_alloc(u64 size); */
/* void* db_realloc(void* ptr, u64 size); */
/* void  db_free(void* ptr); */
/* void* db_arena_free(); */


// 1kb scratch memory
#define MAX_SCRATCH_SIZE 1024 * 1024
#define INVALID_INDEX (u32)(-1)

// Database is set up as a structure of arrays
typedef struct
{

    // --- header start ---
    char passkey[DB_MAX_PWD_STR_SIZE];

    char tags[DB_MAX_TAG_STR_SIZE][sizeof(Tag) * 8]; // 128 possible tags
    u32  tag_count;

    u32             field_count;
    FieldInfo*      fields;

    // ---  header end  ---

    u32             contact_count;
    u32             contact_allocated;
    String*         c_ids;
    Tag*            c_tags;
    FieldData**     c_fields_data;
} Database;

extern Database db;

void db_scratch_init(void);
void db_scratch_uninit(void);
// returns NULL if size exceeds max scratch size
void* db_scratch_alloc(u64 size);

#define ARENA_BLOCK_SIZE 1024
void db_arena_init(void);
void db_arena_uninit(void);
// returns NULL if size exceeds max scratch size
void* db_arena_alloc(u64 size);
void* db_arena_calloc(u64 size, u64 count);
void* db_arena_realloc(void* ptr, u64 prev_size, u64 size);

bool string_compare(String s1, String s2);
StringData* string_to_stringdata(String str);
String string_copy(String str);
void fwstring(String str, FILE* file);
void fwcstring(char* cstr, FILE* file);

#endif
