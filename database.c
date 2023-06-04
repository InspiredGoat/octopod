#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include "database_internal.h"



Database db;

u32 db_index_from_id(String id)
{
    u32 i = 0;
    for (i = 0; i < db.contact_count; i++)
    {
        if (string_compare(id, db.c_ids[i]))
        {
            return i;
        }
    }

    return INVALID_INDEX;
}

void db_init()
{
    db_scratch_init();
    db_arena_init();

    // make sure every value is set to zero
    bzero(&db, sizeof(db));
}
void db_uninit()
{
    db_scratch_uninit();
    db_arena_uninit();

    // make sure every value is set to zero
    bzero(&db, sizeof(db));
}

void db_create(void)
{
    // all values should be zero/NULL to begin with
    bzero(&db, sizeof(db));
    
    // Unencrypted
    db.field_count = 0;
    db.fields      = NULL;

    db.contact_count      = 0;
    db.contact_allocated  = 0;
    db.c_fields_data    = NULL;
    db.c_tags         = NULL;
}


//----------------------------------------------------------------


void db_header_begin(void)
{
    // nothing for now
}
void db_header_add_field(char* name, FieldType type)
{
    db.fields = db_arena_realloc(db.fields, db.field_count, db.field_count + 1);
    memcpy(db.fields[db.field_count].field_name, 
            name, 
            min(strlen(name), DB_MAX_FIELD_STR_SIZE));

    db.fields[db.field_count].type      = type;
    db.fields[db.field_count].hook_type = HOOK_TYPE_NONE;
    db.field_count++;
}
void db_header_add_tag(char* name)
{
    strncpy(db.tags[db.tag_count], name, min(DB_MAX_TAG_STR_SIZE, strlen(name)));

    db.tag_count ++;

    if (db.tag_count >= sizeof(Tag) * 8)
    {
        printf("Ran out of tags!\n");
        assert(0);
    }
}
void db_header_end(void)
{
    // nothing for now
}


//----------------------------------------------------------------


void db_load(char* filename, String passkey)
{
    // XXX: remember on load pointers in fields data are relative to data_raw
}
void db_encrypt(String passkey);

void db_save(char* filename)
{
    FILE* f;

    f = fopen(filename, "wb");
    fwcstring(db.passkey, f);

    fwrite(&db.field_count, sizeof(db.field_count), 1, f);
    for (int i = 0; i < db.field_count; i++)
    {
        fwrite(&db.fields[i], sizeof(FieldInfo), 1, f);
    }

    fwrite(&db.contact_count, sizeof(db.contact_count), 1, f);
    for (int i = 0; i < db.contact_count; i++)
        fwstring(db.c_ids[i], f);

    for (int i = 0; i < db.contact_count; i++)
        fwrite(&db.c_tags[i], sizeof(Tag), 1, f);

    for (int i = 0; i < db.contact_count; i++)
    {
        for (int j = 0; j < db.field_count; j++)
        {
            FieldData** fields = db.c_fields_data;
            fwrite(&fields[i][j].data_size, sizeof(fields[i][j].data_size), 1, f);
            fwrite(fields[i][j].data_raw, fields[i][j].data_size, 1, f);
        }
    }

    fwrite(&db.contact_count, sizeof(db.contact_count), 1, f);

    fclose(f);
}

void db_contact_add(String id)
{
    int i = 0;

    if (db_index_from_id(id) != INVALID_INDEX)
    {
        // value for db exists
        printf("Contact with that name already exists\n");
        assert(0);
    }

    db.contact_count++;
    if (db.contact_count > db.contact_allocated)
    {
        db.contact_allocated = db.contact_count * 2;

        // resize procedure
        db.c_ids            = db_arena_realloc( db.c_ids,
                                               (db.contact_allocated / 2) * sizeof(String), 
                                                db.contact_allocated * sizeof(String));
        db.c_fields_data    = db_arena_realloc( db.c_fields_data,
                                               (db.contact_allocated / 2) * sizeof(FieldData*),
                                                db.contact_allocated * sizeof(FieldData*));
        db.c_tags           = db_arena_realloc( db.c_tags,
                                               (db.contact_allocated / 2) * sizeof(Tag), 
                                                db.contact_allocated * sizeof(Tag));
    }
    i = db.contact_count - 1;

    db.c_ids[i]          = string_copy(id);
    db.c_fields_data[i]  = db_arena_calloc((db.field_count), sizeof(FieldData));
    db.c_tags[i]         = 0;

    for (int i = 0; i < db.field_count; i++)
    {
        // TODO: set field data over here
        FieldInfo info;
    }
    
}
void db_contact_remove(String id)
{
    u32 index = db_index_from_id(id);
    if (index == INVALID_INDEX)
    {
        printf("Name not in database\n");
        assert(0);
    }

    memmove(db.c_fields_data + index,
            db.c_fields_data + index + 1,
            sizeof(FieldData) * (db.contact_count - index - 1));

    db.contact_count --;
}

void db_contact_tag_set(String id, Tag tags)
{
    int index;
    index = db_index_from_id(id);
    if (index == INVALID_INDEX)
    {
        printf("Couldn't find contact id!!\n");
        assert(0);
    }

    db.c_tags[index] = tags;
}
Tag db_contact_tag_get(String id)
{
    int index;
    index = db_index_from_id(id);
    if (index == INVALID_INDEX)
    {
        printf("Couldn't find contact id!!\n");
        assert(0);
    }
    
    return db.c_tags[index];
}


void db_contact_field_set(String id, FieldId field, void* value, u32 value_size)
{
    int index;
    index = db_index_from_id(id);

    if (index == INVALID_INDEX)
    {
        // TODO: handle gracefully
        printf("No contact with that name!\n");
        assert(0);
    }
    else
    {
        FieldData** fields;
        fields = db.c_fields_data;

        if (field > db.field_count)
        {
            // TODO: handle this gracefully
            printf("No such field\n");
            assert(0);
        }

        // failed to find field, must create it instead

        fields[index][field].data_raw = db_arena_realloc(db.c_fields_data[index][field].data_raw, 
                                                         fields[index][field].data_size,
                                                         value_size);
        memcpy(fields[index][field].data_raw, value, value_size);
        fields[index][field].data_size = value_size;
        assert(fields[index][field].data_raw != NULL);

        switch(fields[index][field].type)
        {
            case FIELD_TYPE_INVALID:
                assert(0);
                break;
            case FIELD_TYPE_COUNT:
                assert(0);
                break;
            case FIELD_TYPE_SERVICE_PROFILE:
                fields[index][field].value.service_profile.pubid = fields[index][field].data_raw;
                fields[index][field].value.service_profile.pubname = fields[index][field].data_raw + sizeof(StringData) + fields[index][field].value.service_profile.pubid->length;
                break;
            case FIELD_TYPE_STRING:
                fields[index][field].value.string = fields[index][field].data_raw;
                break;
            case FIELD_TYPE_BYTES:
                fields[index][field].value.bytes = fields[index][field].data_raw;
                break;
            case FIELD_TYPE_U32:
                fields[index][field].value.u = ((u32*)fields[index][field].data_raw);
                break;
            case FIELD_TYPE_I32:
                fields[index][field].value.i = ((i32*)fields[index][field].data_raw);
                break;
        }
    }
}

void db_contact_field_set_string(String id, FieldId field, String str)
{
    StringData* data = string_to_stringdata(str);
    db_contact_field_set(id, field, data, sizeof(StringData) + str.length);
}


FieldData* db_contact_field_get_all(String id)
{
    int index;
    index = db_index_from_id(id);
    if (index == INVALID_INDEX)
    {
        printf("Couldn't find contact id!!\n");
        assert(0);
    }

    return db.c_fields_data[index];
}


char* db_tag_get(Tag bitmask)
{
    for (int i = 0; i < sizeof(bitmask) * 8; i++)
    {
        if ((bitmask & (1 << i)))
        {
            return db.tags[i];
        }
    }

    return NULL;
}

u32 db_fields_count(void)
{
    return db.field_count;
}



void db_search(String query, FieldId field);
void db_search_multi(String query, FieldId* fields, u32 field_count);
