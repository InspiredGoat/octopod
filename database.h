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
    HOOK_TYPE_WEB,
    HOOK_TYPE_SCRIPT,
    HOOK_TYPE_PROGRAM,
    HOOK_TYPE_DLL, // TODO: not implemented
    HOOK_TYPE_COUNT
} HookType;

typedef enum
{
    FIELD_INVALID=-1,
    FIELD_INFO,
    FIELD_SERVICE,
    FIELD_COUNT
} FieldType;

typedef u32 FieldId;

typedef struct
{
    FieldId field_id;   // backend id
    String  field_name; // for rendering purposes so 'Twitter', 'Facebook', 'Telegram'
} FieldInfo;

typedef struct 
{
    FieldInfo info;
    HookType hook_type;
    union
    {
        struct
        {
            String      dm_url;       // url template for instant messaging
            String      feed_url;     // url template to check their "content" (ie their Twitter feed)
        } web;
        struct
        {
            String dm_script;
            String feed_script;
        } script;
        struct
        {
            String program;
            u32 argument_count;
            String* arguments;
        } program;
        struct
        {
            // TODO: find way to implement
        } dll;
    } hook_data;
} Service;

typedef struct
{
    union
    {
        // social media sites usually store an @ name and an internal id
        // to interface with these sites we need both the @ and the id
        struct
        {
            StringData*  pubname;
            StringData*  pubid;
        } service_profile;
        StringData* value;
        void* raw_data;
    } value;
} FieldContact;


typedef u128 Tag;

// Database is set up as a structure of arrays
typedef struct
{
    String passkey;
    String tags[sizeof(Tag) * 8]; // 128 possible tags

    u32             field_count;
    FieldInfo*      fields;

    u32             service_count;
    Service*        services;

    u32             contact_count;
    u32             contact_allocated;
    String*         c_ids;
    u32*            c_field_counts;
    FieldContact**  c_field_data;
    Tag*            c_tags;
} Database;

void db_init(void);
void db_create(void);
void db_load(char* filename, String passkey);
void db_encrypt(String passkey);
void db_save(char* filename);
void db_uninit(void);

void db_contact_add(String id);
// string value is copied into database, feel free to free after
void db_contact_field_set(String id, FieldId field, void* value, u32 value_size);

void db_contact_tag_set(String id, Tag tags);
void db_contact_remove(String name_id);

void db_search(String query, FieldId field);
void db_search_multi(String query, FieldId* fields, u32 field_count);


//------------------------------------------------------------


#endif
