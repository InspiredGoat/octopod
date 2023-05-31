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


typedef struct
{
    u32 length;
    char* data;
} String;


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

typedef String FieldId;

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
            String argument_count;
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
    FieldId id;
    String  pubname;
    String  pubid;
} FieldContact;


typedef u128 Tag;

// Database is set up as a structure of arrays
typedef struct
{
    String tags[sizeof(Tag) * 8]; // 128 possible tags

    u32             field_count;
    FieldInfo*      fields;

    u32             service_count;
    Service*        services;

    u32             contact_count;
    int*            c_field_counts;
    FieldContact**  c_field_ids;
    Tag*            c_tags;
} Database;

void db_load(char* filename, char* passkey);
void db_encrypt(char* passkey);
void db_save();

void db_contact_add(char* name_id);
void db_contact_set(char* name_id);
void db_contact_remove(char* name_id);

String db_search(String query, FieldId field);
String db_search_multiple(String query, FieldId* fields, u32 field_count);


//------------------------------------------------------------


#endif
