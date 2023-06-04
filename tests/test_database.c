#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include <string.h>

#include "../database.h"

const String null = {
    0, NULL
};


static void test_add_contacts(void **state)
{
    db_init();
    db_create();

    String name;
    name.data = "richard";
    name.length = 7;
    db_contact_add(name);
    name.data = "rihard";
    name.length = 6;
    db_contact_add(name);
    name.data = "eihard";
    name.length = 6;
    db_contact_add(name);
    name.data = "ebhard";
    name.length = 6;
    db_contact_add(name);
    name.data = "ebharq";
    name.length = 6;
    db_contact_add(name);

    name.data = "eihard";
    name.length = 6;
    db_contact_tag_set(name, 0xffffffffffffffff);

    db_save("out.db");
    db_uninit();
}

static void test_checkfields(void **state)
{
    db_init();
    db_create();

    db_header_begin();

    db_header_add_field("First Name",   FIELD_TYPE_STRING);
    db_header_add_field("Last Name",    FIELD_TYPE_STRING);
    db_header_add_field("Phone Number", FIELD_TYPE_STRING);
    db_header_add_field("Facebook",     FIELD_TYPE_SERVICE_PROFILE);

    db_header_add_tag(cstr_to_string("stranger"));
    db_header_add_tag(cstr_to_string("friend"));
    db_header_add_tag(cstr_to_string("family"));
    db_header_add_tag(cstr_to_string("coworker"));
    db_header_add_tag(cstr_to_string("infidel"));

    db_header_end();

    String c1 = cstr_to_string("richard");
    db_contact_add(c1);
    db_contact_add(cstr_to_string("bob"));

    db_contact_tag_set(c1, 0xffffffffffffffff);
    FieldData* fields = db_contact_field_get_all(c1);

    db_contact_field_set_string(c1, 0, cstr_to_string("Richard"));
    db_contact_field_set_string(c1, 1, cstr_to_string("Lennox"));
    /* for (int i = 0; i < ) */
    /* { */
        
    /* } */

    FieldData* data = db_contact_field_get_all(cstr_to_string("richard"));

    assert_string_equal(data[0].value.string->data, "Richard");
    assert_string_equal(data[1].value.string->data, "Lennox");


    db_save("out.db");

    db_uninit();
}


int main()
{

    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_add_contacts),
        cmocka_unit_test(test_checkfields),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}


