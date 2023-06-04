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

void db_wire_field_data(FieldData* field_data)
{
    switch(field_data->type)
    {
        case FIELD_TYPE_INVALID:
            assert(0);
            break;
        case FIELD_TYPE_COUNT:
            assert(0);
            break;
        case FIELD_TYPE_SERVICE_PROFILE:
            field_data->value.service_profile.pubid = field_data->data_raw;
            field_data->value.service_profile.pubname = field_data->data_raw + sizeof(StringData) + field_data->value.service_profile.pubid->length;
            break;
        case FIELD_TYPE_STRING:
            field_data->value.string = field_data->data_raw;
            break;
        case FIELD_TYPE_BYTES:
            field_data->value.bytes = field_data->data_raw;
            break;
        case FIELD_TYPE_U32:
            field_data->value.u = ((u32*)field_data->data_raw);
            break;
        case FIELD_TYPE_I32:
            field_data->value.i = ((i32*)field_data->data_raw);
            break;
    }
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

u32 db_string_hash(String password)
{
    u32 hash;
    hash = 0;
    for (u32 i = 0; i < password.length; i++)
    {
        char c = password.data[i];
        hash = (hash * 10) + (c - '0');
    }

    return hash;
}

static FILE* db_file;
static u32 db_file_off;
static u32 db_file_start_hash;

void db_read_begin(char* filename, String password)
{
    db_file = fopen(filename, "rb");
    db_file_start_hash = db_string_hash(password);
    db_file_off = 0;
    srand(db_file_start_hash);
}
void db_read(void* ret_buf, u32 size)
{
    for (u32 i = 0; i < size; i++)
    {
        byte b;
        fread(&b, 1, 1, db_file);

        srand(db_file_start_hash ^ db_file_off);
        b = b ^ (rand() % 255);

        ((byte*)ret_buf)[i] = b;

        db_file_off++;
    }
}
void db_read_end()
{
    fclose(db_file);
}

void db_write_begin(char* filename, String password)
{
    db_file = fopen(filename, "wb");
    db_file_start_hash = db_string_hash(password);
    db_file_off = 0;
    srand(db_file_start_hash);
}
void db_write(void* ret_buf, u32 size)
{
    for (u32 i = 0; i < size; i++)
    {
        byte b = ((byte*)ret_buf)[i];

        srand(db_file_start_hash ^ db_file_off);
        b = b ^ (rand() % 255);
        
        fwrite(&b, 1, 1, db_file);

        db_file_off++;
    }
}
void db_write_end()
{
    fclose(db_file);
}

void db_load(char* filename, String password)
{
    db_read_begin(filename, password);

    db_read(&db.field_count, sizeof(db.field_count));
    db.fields = db_arena_calloc(sizeof(FieldInfo), db.field_count);
    for (int i = 0; i < db.field_count; i++)
    {
        db_read(&db.fields[i], sizeof(FieldInfo));
    }

    db_read(&db.contact_count, sizeof(db.contact_count));
    db.c_ids = db_arena_calloc(sizeof(String), db.contact_count);
    for (int i = 0; i < db.contact_count; i++)
    {
        db_read(&db.c_ids[i].length, sizeof(db.c_ids[i].length));

        db.c_ids[i].data = db_arena_alloc(db.c_ids[i].length);
        db_read(db.c_ids[i].data, db.c_ids[i].length);
    }

    db.c_tags = db_arena_calloc(sizeof(Tag), db.contact_count);
    for (int i = 0; i < db.contact_count; i++)
        db_read(&db.c_tags[i], sizeof(Tag));

    db.c_fields_data = db_arena_calloc(sizeof(FieldData*), db.contact_count);
    for (int i = 0; i < db.contact_count; i++)
    {
        db.c_fields_data[i] = db_arena_calloc(sizeof(FieldData), db.field_count);
        for (int j = 0; j < db.field_count; j++)
        {
            FieldData** fields = db.c_fields_data;
            db_write(&fields[i][j], sizeof(fields[i][j]));

            fields[i][j].data_raw = db_arena_alloc(fields[i][j].data_size);
            db_read(fields[i][j].data_raw, fields[i][j].data_size);

            db_wire_field_data(&fields[i][j]);
            fields[i][j].value.string = fields[i][j].data_raw;
            fields[i][j].value.service_profile.pubid    = fields[i][j].data_raw;
            fields[i][j].value.service_profile.pubname  = fields[i][j].data_raw + 
                                                          sizeof(StringData)    +
                                                          fields[i][j].value.service_profile.pubname->length;
        }
    }

    db_read_end();
}

void db_save(char* filename, String password)
{
    db_write_begin(filename, password);

    db_write(&db.field_count, sizeof(db.field_count));

    for (int i = 0; i < db.field_count; i++)
    {
        db_write(&db.fields[i], sizeof(FieldInfo));
    }

    db_write(&db.contact_count, sizeof(db.contact_count));
    for (int i = 0; i < db.contact_count; i++)
    {
        db_write(&db.c_ids[i].length, sizeof(db.c_ids[i].length));
        db_write(db.c_ids[i].data, db.c_ids[i].length);
    }

    for (int i = 0; i < db.contact_count; i++)
        db_write(&db.c_tags[i], sizeof(Tag));

    for (int i = 0; i < db.contact_count; i++)
    {
        for (int j = 0; j < db.field_count; j++)
        {
            FieldData** fields = db.c_fields_data;
            db_write(&fields[i][j], sizeof(fields[i][j]));
            db_write(fields[i][j].data_raw, fields[i][j].data_size);
        }
    }

    db_write_end();
}

//----------------------------------------------------------------

bool db_contact_add(String id)
{
    int i = 0;

    if (db_index_from_id(id) != INVALID_INDEX)
    {
        // value for db exists
        printf("Contact with that name already exists\n");
        return FALSE;
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
    
    return TRUE;
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


u32 db_contacts_count()
{
    return db.contact_count;
}

String db_contact_id_from_index(u32 index)
{
    assert(index < db.contact_count && index < db.contact_allocated);

    return string_temp_copy(db.c_ids[index]);
}

StringData** db_contacts_ids_tempalloc()
{
    StringData** ret;
    ret = db_scratch_alloc(db.contact_count * sizeof(StringData*));
    for (u32 i = 0; i < db.contact_count; i++)
    {
        ret[i] = db_scratch_alloc(sizeof(StringData) + db.c_ids[i].length);
        memcpy( &ret[i]->length, 
                &db.c_ids[i].length, 
                sizeof(db.c_ids[i].length));
        memcpy( &ret[i]->data,
                db.c_ids[i].data,
                db.c_ids[i].length);
    }

    return ret;
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

        db_wire_field_data(&fields[index][field]);
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
