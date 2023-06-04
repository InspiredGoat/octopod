#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <cmocka.h>

#include <string.h>

#include "../database.h"
#include "../database_internal.h"

const String null = {
    0, NULL
};

static void assert_str(char* s1, char* s2, u32 n)
{
    bool b;
    b = (strncmp(s1, s2, n) == 0);

    if (!b)
    {
        for (u32 i = 0; i < n; i++)
            putchar(s1[i]);

        printf(" != ");

        for (u32 i = 0; i < n; i++)
            putchar(s2[i]);

        printf("\n");

        assert_true(0);
    }
}


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

    db_save("out.db", cstr_to_string(""));
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

    db_save("out.db", cstr_to_string(""));

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

static void test_encrypt(void** state)
{
    typedef struct
    {
        char    fname[32];
        char    lname[32];
        u32     age;
        u32     cash;
    } TestDataStruct;

    db_write_begin("testfile.txt", cstr_to_string("password"));

    TestDataStruct w1;
    strcpy(w1.fname, "Michel");
    strcpy(w1.lname, "Douglas");
    w1.age = 39;
    w1.cash = 54000;
    db_write(&w1, sizeof(w1));

    TestDataStruct w2;
    strcpy(w2.fname, "Bobby");
    strcpy(w2.lname, "Johnson");
    w2.age = 82;
    w2.cash = 900000;
    db_write(&w2, sizeof(w2));

    TestDataStruct w3;
    strcpy(w3.fname, "Bob");
    strcpy(w3.lname, "Smith");
    w3.age = 11;
    w3.cash = 200;
    db_write(&w3, sizeof(w3));

    db_write_end();

    db_read_begin("testfile.txt", cstr_to_string("password"));
    TestDataStruct r1;
    db_read(&r1, sizeof(r1));

    TestDataStruct r2;
    db_read(&r2, sizeof(r2));

    TestDataStruct r3;
    db_read(&r3, sizeof(r3));
    db_read_end();

    assert_string_equal(w1.fname, r1.fname);
    assert_string_equal(w2.fname, r2.fname);
    assert_string_equal(w3.fname, r3.fname);

    assert_string_equal(w1.lname, r1.lname);
    assert_string_equal(w2.lname, r2.lname);
    assert_string_equal(w3.lname, r3.lname);

    assert_int_equal(w1.age, r1.age);
    assert_int_equal(w2.age, r2.age);
    assert_int_equal(w3.age, r3.age);

    assert_int_equal(w1.cash, r1.cash);
    assert_int_equal(w2.cash, r2.cash);
    assert_int_equal(w3.cash, r3.cash);

    // changing password should result in encryption failing

    db_read_begin("testfile.txt", cstr_to_string("pssword"));
    db_read(&r1, sizeof(r1));
    db_read(&r2, sizeof(r2));
    db_read(&r3, sizeof(r3));
    db_read_end();

    assert_string_not_equal(w1.fname, r1.fname);
    assert_string_not_equal(w2.fname, r2.fname);
    assert_string_not_equal(w3.fname, r3.fname);

    assert_string_not_equal(w1.lname, r1.lname);
    assert_string_not_equal(w2.lname, r2.lname);
    assert_string_not_equal(w3.lname, r3.lname);

    assert_int_not_equal(w1.age, r1.age);
    assert_int_not_equal(w2.age, r2.age);
    assert_int_not_equal(w3.age, r3.age);

    assert_int_not_equal(w1.cash, r1.cash);
    assert_int_not_equal(w2.cash, r2.cash);
    assert_int_not_equal(w3.cash, r3.cash);
}

static void test_save_and_load(void** state)
{
    db_init();
    db_create();

    db_header_begin();
    db_header_add_tag("friend");
    db_header_add_tag("family");
    db_header_add_tag("close");
    db_header_add_tag("irl");
    db_header_add_tag("online");
    db_header_add_tag("client");

    db_header_add_field("fname", FIELD_TYPE_STRING);
    db_header_add_field("lname", FIELD_TYPE_STRING);
    db_header_add_field("email", FIELD_TYPE_STRING);
    db_header_add_field("age",   FIELD_TYPE_U32);
    db_header_end();

    String c1, c2, c3;
    u32 val;
    c1 = cstr_to_string("bob");
    db_contact_add(c1);
    db_contact_field_set_string(c1, 0, cstr_to_string("Bob"));
    db_contact_field_set_string(c1, 1, cstr_to_string("Lasso"));
    db_contact_field_set_string(c1, 2, cstr_to_string("realboblas@gmail.com"));
    val = 32;

    db_contact_field_set(c1, 3, &val, sizeof(val));
    db_contact_tag_set(c1, 0b101011);
    
    c2 = cstr_to_string("lucas");
    db_contact_add(c2);
    db_contact_tag_set(c2, 0b011100);
    db_contact_field_set_string(c2, 0, cstr_to_string("Lucas"));
    db_contact_field_set_string(c2, 1, cstr_to_string("Smith"));
    db_contact_field_set_string(c2, 2, cstr_to_string("lucas@lucassmith.xyz"));

    c3 = cstr_to_string("mitchel");
    db_contact_add(c3);
    db_contact_tag_set(c3, 0b101011);
    db_contact_field_set_string(c3, 0, cstr_to_string("Mitchel"));
    db_contact_field_set_string(c3, 1, cstr_to_string("Donald"));
    db_contact_field_set_string(c3, 2, cstr_to_string("mitchdonal@hotmail.com"));

    db_save("testdb.db", cstr_to_string("supersecretpassword"));

    db_uninit();

    // try to load

    db_init();

    db_load("testdb.db", cstr_to_string("supersecretpassword"));

    FieldData* data;
    data = db_contact_field_get_all(c1);

    assert_str(data[0].value.string->data, "Bob",       min(3, data[0].value.string->length));
    assert_str(data[1].value.string->data, "Lasso",     min(5, data[1].value.string->length));
    assert_str(data[2].value.string->data, "realboblas@gmail.com", min(20, data[2].value.string->length));

    data = db_contact_field_get_all(c2);
    assert_str(data[0].value.string->data, "Lucas",     min(5, data[0].value.string->length));
    assert_str(data[1].value.string->data, "Smith",     min(5, data[1].value.string->length));
    assert_str(data[2].value.string->data, "realboblas@gmail.com", min(20, data[2].value.string->length));

    data = db_contact_field_get_all(c3);
    assert_str(data[0].value.string->data, "Mitchel",   min(7, data[0].value.string->length));
    assert_str(data[1].value.string->data, "Donald",    min(6, data[1].value.string->length));
    assert_str(data[2].value.string->data, "mitchdonal@hotmail.com", min(22, data[2].value.string->length));

    db_uninit();
}



int main()
{

    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_add_contacts),
        cmocka_unit_test(test_setfields),
        cmocka_unit_test(test_tags),
        cmocka_unit_test(test_encrypt),
        cmocka_unit_test(test_save_and_load),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

