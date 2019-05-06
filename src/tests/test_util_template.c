/*
    Authors:
        Tomas Halman <thalman@redhat.com>

    Copyright (C) 2019 Red Hat

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tests/test_common.h"
#include "lib/util/string_array.h"
#include "lib/util/template.h"

void test_template_if(void **state)
{
    const char *myfeatures[] = {
        "true",
        NULL
    };

    const char *template =
        "line 01 {if \"true\":yes}  \n"
        "line 02 {if \"false\":yes}  \n"
        "line 03 {if \"true\":yes|no}  \n"
        "line 04 {if \"false\":yes|no}  \n"
        "";
    const char *expected =
        "line 01 yes\n"
        "line 02\n"
        "line 03 yes\n"
        "line 04 no\n"
        "";

    char *result = template_generate(template, myfeatures);
    assert_string_equal(expected, result);
    free(result);
}

void test_template_if_not(void **state)
{
    const char *myfeatures[] = {
        "true",
        NULL
    };

    const char *template =
        "line 01 {if not \"true\":yes}  \n"
        "line 02 {if not \"false\":yes}  \n"
        "line 03 {if not \"true\":yes|no}  \n"
        "line 04 {if not \"false\":yes|no}  \n"
        "";
    const char *expected =
        "line 01\n"
        "line 02 yes\n"
        "line 03 no\n"
        "line 04 yes\n"
        "";

    char *result = template_generate(template, myfeatures);
    assert_string_equal(expected, result);
    free(result);
}

void test_template_exclude(void **state)
{
    const char *myfeatures[] = {
        "true",
        NULL
    };

    const char *template =
        "line 01 {exclude if \"true\"}\n"
        "line 02 {exclude if \"false\"}\n"
        "line 03\n"
        "";
    const char *expected =
        "line 02\n"
        "line 03\n"
        "";

    char *result = template_generate(template, myfeatures);
    assert_string_equal(expected, result);
    free(result);
}

void test_template_include(void **state)
{
    const char *myfeatures[] = {
        "true",
        NULL
    };

    const char *template =
        "line 01 {include if \"true\"}\n"
        "line 02 {include if \"false\"}\n"
        "line 03\n"
        "";
    const char *expected =
        "line 01\n"
        "line 03\n"
        "";

    char *result = template_generate(template, myfeatures);
    assert_string_equal(expected, result);
    free(result);
}


void test_template_stop(void **state)
{
    const char *myfeatures[] = {
        "true",
        NULL
    };

    const char *template =
        "line 01\n"
        "{stop if \"false\"}\n"
        "line 02\n"
        "{stop if \"true\"}\n"
        "line 03\n"
        "";
    const char *expected =
        "line 01\n"
        "line 02\n"
        "";

    char *result = template_generate(template, myfeatures);
    assert_string_equal(expected, result);
    free(result);
}

void test_template_continue(void **state)
{
    const char *myfeatures[] = {
        "true",
        NULL
    };

    const char *template =
        "line 01\n"
        "{continue if \"true\"}\n"
        "line 02\n"
        "{continue if \"false\"}\n"
        "line 03\n"
        "";
    const char *expected =
        "line 01\n"
        "line 02\n"
        "";

    char *result = template_generate(template, myfeatures);
    assert_string_equal(expected, result);
    free(result);
}

int main(int argc, const char *argv[])
{

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_template_if),
        cmocka_unit_test(test_template_if_not),
        cmocka_unit_test(test_template_include),
        cmocka_unit_test(test_template_exclude),
        cmocka_unit_test(test_template_stop),
        cmocka_unit_test(test_template_continue),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
