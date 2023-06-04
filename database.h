#ifndef DATABASE_H
#define DATABASE_H

#include <stdint.h>

typedef unsigned char   byte;
typedef int             i8;
typedef short           i16;
typedef int             i32;
typedef long            i64;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long       u64;
typedef __uint128_t         u128; // TODO: check this is portable

#ifndef __cplusplus
#ifndef bool
typedef unsigned char bool;
#endif
#endif

#define TRUE  1
#define FALSE 0

#define DB_MAX_CID_STR_SIZE 128
#define DB_MAX_TAG_STR_SIZE 128
#define DB_MAX_PWD_STR_SIZE 128
#define DB_MAX_URL_STR_SIZE 1024
#define DB_MAX_FIELD_STR_SIZE 128


typedef struct
{
    u32 length;
    char* data;
} String;

typedef struct
{
    u32 length;
    char data[];
} StringData;


typedef enum
{
    HOOK_TYPE_INVALID=-1,
    HOOK_TYPE_NONE,
    HOOK_TYPE_WEB,
    HOOK_TYPE_SCRIPT,
    HOOK_TYPE_PROGRAM,
    HOOK_TYPE_DLL, // TODO: not implemented
    HOOK_TYPE_COUNT
} HookType;

/* typedef enum */
/* { */
/*     FIELD_INVALID=-1, */
/*     FIELD_INFO, */
/*     FIELD_SERVICE, */
/*     FIELD_COUNT */
/* } FieldType; */

typedef u32 FieldId;

typedef enum
{
    FIELD_TYPE_INVALID=-1,
    FIELD_TYPE_STRING,
    FIELD_TYPE_SERVICE_PROFILE,
    FIELD_TYPE_U32,
    FIELD_TYPE_I32,
    FIELD_TYPE_BYTES,
    FIELD_TYPE_COUNT
} FieldType;

typedef struct FieldInfo
{
    char  field_name[DB_MAX_FIELD_STR_SIZE]; // for rendering purposes so 'Twitter', 'Facebook', 'Telegram'
    FieldType type;
    HookType hook_type;

    union
    {
        struct
        {
            char      dm_url[DB_MAX_URL_STR_SIZE];       // url template for instant messaging
            char      feed_url[DB_MAX_URL_STR_SIZE];     // url template to check their "content" (ie their Twitter feed)
        } web;
        struct
        {
            char dm_script[DB_MAX_URL_STR_SIZE];
            char feed_script[DB_MAX_URL_STR_SIZE];
        } script;
        struct
        {
            char program_cmd[DB_MAX_URL_STR_SIZE];
        } program;
        struct
        {
            // TODO: find way to implement
        } dll;
    } hook_data;
} FieldInfo;

typedef struct
{
    FieldType type;
    union
    {
        // social media sites usually store an @ name and an internal id
        // to interface with these sites we need both the @ and the id
        struct
        {
            StringData*  pubname;
            StringData*  pubid;
        } service_profile;
        StringData* string;
        byte* bytes;
        u32* u;
        i32* i;
    } value;
    u32     data_size;
    void*   data_raw;
} FieldData;

typedef u128 Tag;

void db_init(void);
void db_uninit(void);
void db_create(void);

// reset database header
void db_header_begin(void);
void db_header_add_tag(char* name);
void db_header_add_field(char* name, FieldType type);
void db_header_add_field_hook_web(char* name, FieldType type, char* dm_url, char* feed_url);
void db_header_add_field_hook_script(char* name, FieldType type, char* dm_script, char* feed_script);
void db_header_add_field_hook_program(char* name, FieldType type, char* cmd);
void db_header_end(void);

// gets the string for the first bit in mask
char* db_tag_get(Tag bitmask);


void db_load(char* filename, String passkey);
void db_encrypt(String passkey);
void db_save(char* filename);

void db_contact_add(String id);
void db_contact_remove(String name_id);

u32 db_fields_count(void);

// value is copied, free it on your end
void db_contact_field_set(String id, FieldId field, void* value, u32 value_size);
/* void db_contact_field_set(String id, FieldId field, FieldType type, void* value, u32 value_size); */
void db_contact_field_set_string(String id, FieldId field, String str);

void db_contact_tag_set(String id, Tag tags);

Tag db_contact_tag_get(String name_id);
FieldData* db_contact_field_get_all(String name_id);

void db_search(String query, FieldId field);
void db_search_multi(String query, FieldId* fields, u32 field_count);


//------------------------------------------------------------


u32 min(u32 a, u32 b);
u32 max(u32 a, u32 b);

String cstr_to_string(char* str);

#endif
