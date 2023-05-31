#ifndef DATABASE_H
#define DATABASE_H

typedef unsigned char  byte;
typedef int   i8;
typedef short i16;
typedef int   i32;
typedef long  i64;
typedef unsigned char  u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef unsigned long   u64;

typedef struct
{
    u32 length;
    char* data;
} String;

typedef String ProviderId;

typedef enum
{
    HOOK_TYPE_INVALID=-1,
    HOOK_TYPE_WEB,
    HOOK_TYPE_SCRIPT,
    HOOK_TYPE_PROGRAM,
    HOOK_TYPE_COUNT
} HookType;

typedef struct
{
    ProviderId id;
    String pubname;
    String pubid;
} ProviderContact;

typedef struct
{
    ProviderId  provider_id;  // human recognizable for scripting could be 'twitter', 'fb', 'telegram'
    String      display_name; // for rendering purposes so 'Twitter', 'Facebook', 'Telegram'
} Info;

typedef struct 
{
    Info info;
    ProviderId  provider_id;  // human recognizable for scripting could be 'twitter', 'fb', 'telegram'
    String      display_name; // for rendering purposes so 'Twitter', 'Facebook', 'Telegram'
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
            String script;
        } script;
        struct 
        {
            String program;
            String argument_count;
            String* arguments;
        } program;
    } hook_data;
} Provider;


// Database is set up as a structure of arrays
typedef struct
{
    Provider contact;
} ProvidersSupported;
typedef struct
{
    u32 own_provider_count;
    Provider* own_providers;
    u32 contact_count;
    String* c_ids;
    String* c_alias;
    String* c_f_names;
    String* c_l_names;
    String* c_phones;
    String* c_websites;
    int*    provider_counts;
    ProviderId** provider_ids;
} Database;

void db_load(char* filename, char* passkey);
void db_encrypt(char* passkey);
void db_save();

void db_contact_add(char* name_id);
void db_contact_set(char* name_id);
void db_contact_remove(char* name_id);

String db_search(String query, ProviderId field);

//------------------------------------------------------------


#endif
