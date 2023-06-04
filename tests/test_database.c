#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
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

static void test_setfields(void **state)
{
    db_init();
    db_create();

    db_header_begin();

    db_header_add_field("First Name",   FIELD_TYPE_STRING);
    db_header_add_field("Last Name",    FIELD_TYPE_STRING);
    db_header_add_field("Phone Number", FIELD_TYPE_STRING);
    db_header_add_field("Facebook",     FIELD_TYPE_SERVICE_PROFILE);
    db_header_end();

    String c1 = cstr_to_string("richard");
    db_contact_add(c1);
    db_contact_add(cstr_to_string("bob"));

    db_contact_tag_set(c1, 0xffff);
    FieldData* fields = db_contact_field_get_all(c1);

    db_contact_field_set_string(c1, 0, cstr_to_string("Richard"));
    db_contact_field_set_string(c1, 1, cstr_to_string("Lennox"));

    FieldData* data = db_contact_field_get_all(cstr_to_string("richard"));
    assert_false(strncmp(data[0].value.string->data, "Richard", strlen("Richard")));
    assert_false(strncmp(data[1].value.string->data, "Lennox",  strlen("Lennox")));

    db_save("out.db");

    db_uninit();
}

static void test_tags(void** state)
{
    db_init();
    db_create();

    db_header_begin();

    db_header_add_tag("stranger");
    db_header_add_tag("doctor");
    db_header_add_tag("friend");
    db_header_add_tag("family");
    db_header_add_tag("coworker");
    db_header_add_tag("infidel");

    db_header_end();

    String c1 = cstr_to_string("richard");
    db_contact_add(c1);

    db_contact_tag_set(c1, 0b111101);
    Tag t = db_contact_tag_get(c1);

    // he's tagged with everything except for doctor
    char* correct_tags[] = 
    {
        "stranger",
        NULL,
        "friend",
        "family",
        "coworker",
        "infidel"
    };

    for (int i = 0; i < 5; i++)
    {
        if (t & (1 << i))
        {
            char* name = db_tag_get((1 << i));

            assert_string_equal(name, correct_tags[i]);
        }
    }

    db_uninit();
}


int main()
{

    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_add_contacts),
        cmocka_unit_test(test_setfields),
        cmocka_unit_test(test_tags),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

