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
        "line 05 {if \"false\" or \"true\":yes|no}  \n"
        "line 06 {if \"false\" and \"true\":yes|no}  \n"
        "line 07 {if \"true\" and (\"true\" or \"false\"):yes|no}  \n"
        "line 08 {if \"false\" or (\"true\" and \"false\"):yes|no}  \n"
        "";
    const char *expected =
        "line 01 yes\n"
        "line 02\n"
        "line 03 yes\n"
        "line 04 no\n"
        "line 05 yes\n"
        "line 06 no\n"
        "line 07 yes\n"
        "line 08 no\n"
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
        "line 05 {if not \"false\" and \"true\":yes|no}  \n"
        "line 06 {if not \"true\" and \"false\":yes|no}  \n"
        "line 07 {if not (\"false\" and \"true\"):yes|no}  \n"
        "";
    const char *expected =
        "line 01\n"
        "line 02 yes\n"
        "line 03 no\n"
        "line 04 yes\n"
        "line 05 yes\n"
        "line 06 no\n"
        "line 07 yes\n"
        "";

    char *result = template_generate(template, myfeatures);
    assert_string_equal(expected, result);
    free(result);
}

void test_template_exclude_if(void **state)
{
    const char *myfeatures[] = {
        "true",
        NULL
    };

    const char *template =
        "line 01 {exclude if \"true\"}\n"
        "line 02 {exclude if \"false\"}\n"
        "line 03 {exclude if \"false\" or \"true\"}\n"
        "line 04 {exclude if \"false\" and \"true\"}\n"
        "";
    const char *expected =
        "line 02\n"
        "line 04\n"
        "";

    char *result = template_generate(template, myfeatures);
    assert_string_equal(expected, result);
    free(result);
}

void test_template_include_if(void **state)
{
    const char *myfeatures[] = {
        "true",
        NULL
    };

    const char *template =
        "line 01 {include if \"true\"}\n"
        "line 02 {include if \"false\"}\n"
        "line 03 {include if \"false\" or \"true\"}\n"
        "line 04 {include if \"false\" and \"true\"}\n"
        "";
    const char *expected =
        "line 01\n"
        "line 03\n"
        "";

    char *result = template_generate(template, myfeatures);
    assert_string_equal(expected, result);
    free(result);
}


void test_template_stop_if(void **state)
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

    const char *template2 =
        "line 01\n"
        "{stop if \"false\" and \"true\"}\n"
        "line 02\n"
        "{stop if \"true\" or \"false\"}\n"
        "line 03\n"
        "";
    const char *expected2 =
        "line 01\n"
        "line 02\n"
        "";

    char *result = template_generate(template, myfeatures);
    assert_string_equal(expected, result);
    free(result);

    result = template_generate(template2, myfeatures);
    assert_string_equal(expected2, result);
    free(result);
}

void test_template_continue_if(void **state)
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

    const char *template2 =
        "line 01\n"
        "{continue if \"true\" or \"false\"}\n"
        "line 02\n"
        "{continue if \"true\" and \"false\"}\n"
        "line 03\n"
        "";
    const char *expected2 =
        "line 01\n"
        "line 02\n"
        "";

    char *result = template_generate(template, myfeatures);
    assert_string_equal(expected, result);
    free(result);

    result = template_generate(template2, myfeatures);
    assert_string_equal(expected2, result);
    free(result);
}


void test_template_list_features(void **state)
{
    int i;
    const char *template =
        "line 01 {if \"feature1\":yes}   \n"
        "line 02 {if \"feature2\":yes|no}\n"
        "line 03 {if \"feature3\" or \"feature1\":yes}   \n"
        "line 04 {if not \"feature4\" or \"feature1\":yes|no}\n"
        "{stop if \"feature5\" or \"feature6\"}\n"
        "{continue if \"feature1\" and (\"feature2\" or \"feature7\")}\n"
        "";
    const char *expected[] = {
        "feature1",
        "feature2",
        "feature3",
        "feature4",
        "feature5",
        "feature6",
        "feature7",
        NULL
    };
    char **flist;

    flist = template_list_features(template);
    assert_non_null(flist);

    assert_int_equal(string_array_count(flist),
                     string_array_count((char **)expected));

    string_array_sort(flist);
    for (i = 0; i < string_array_count(flist); ++i) {
        assert_string_equal(expected[i], flist[i]);
    }
}

int main(int argc, const char *argv[])
{

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_template_if),
        cmocka_unit_test(test_template_if_not),
        cmocka_unit_test(test_template_include_if),
        cmocka_unit_test(test_template_exclude_if),
        cmocka_unit_test(test_template_stop_if),
        cmocka_unit_test(test_template_continue_if),
        cmocka_unit_test(test_template_list_features),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
