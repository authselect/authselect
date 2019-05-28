/*
    Authors:
        Pavel Březina <pbrezina@redhat.com>

    Copyright (C) 2018 Red Hat

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

#include <stdbool.h>
#include <string.h>

#include "tests/test_common.h"
#include "lib/util/string_array.h"

void test_string_array_create(void **state)
{
    char **array;

    array = string_array_create(0);
    assert_non_null(array);
    assert_null(array[0]);

    string_array_free(array);
}

void test_string_array_del_value__single(void **state)
{
    char **array;
    const char *values[] = {"1", "2", "3", "4", "5", NULL};
    const char *expected[] = {"1", "3", "4", "5", NULL};
    int i;

    array = string_array_create(10);
    assert_non_null(array);

    /* Fill array. */
    for (i = 0; values[i] != NULL; i++) {
        array = string_array_add_value(array, values[i], false);
        assert_non_null(array);
        assert_non_null(array[i]);
    }
    assert_null(array[i]);

    /* Delete value. */
    string_array_del_value(array, "2");

    /* Test values. */
    for (i = 0; expected[i] != NULL; i++) {
        assert_non_null(array[i]);
        assert_string_equal(array[i], expected[i]);
    }
    assert_null(array[i]);

    string_array_free(array);
}

void test_string_array_del_value__single_repeated(void **state)
{
    char **array;
    const char *values[] = {"1", "2", "2", "3", "2", "4", "2", "5", NULL};
    const char *expected[] = {"1", "3", "4", "5", NULL};
    int i;

    array = string_array_create(10);
    assert_non_null(array);

    /* Fill array. */
    for (i = 0; values[i] != NULL; i++) {
        array = string_array_add_value(array, values[i], false);
        assert_non_null(array);
        assert_non_null(array[i]);
    }
    assert_null(array[i]);

    /* Delete value. */
    string_array_del_value(array, "2");

    /* Test values. */
    for (i = 0; expected[i] != NULL; i++) {
        assert_non_null(array[i]);
        assert_string_equal(array[i], expected[i]);
    }
    assert_null(array[i]);

    string_array_free(array);
}

void test_string_array_del_value__multiple(void **state)
{
    char **array;
    const char *values[] = {"1", "2", "3", "4", "5", NULL};
    const char *expected[] = {"1", "4", NULL};
    int i;

    array = string_array_create(10);
    assert_non_null(array);

    /* Fill array. */
    for (i = 0; values[i] != NULL; i++) {
        array = string_array_add_value(array, values[i], false);
        assert_non_null(array);
        assert_non_null(array[i]);
    }
    assert_null(array[i]);

    /* Delete value. */
    string_array_del_value(array, "2");
    string_array_del_value(array, "3");
    string_array_del_value(array, "5");

    /* Test values. */
    for (i = 0; expected[i] != NULL; i++) {
        assert_non_null(array[i]);
        assert_string_equal(array[i], expected[i]);
    }
    assert_null(array[i]);

    string_array_free(array);
}

void test_string_array_del_value__multiple_repeated(void **state)
{
    char **array;
    const char *values[] = {"1", "2", "2", "3", "3", "2", "4", "2", "5", "5", NULL};
    const char *expected[] = {"1", "4", NULL};
    int i;

    array = string_array_create(10);
    assert_non_null(array);

    /* Fill array. */
    for (i = 0; values[i] != NULL; i++) {
        array = string_array_add_value(array, values[i], false);
        assert_non_null(array);
        assert_non_null(array[i]);
    }
    assert_null(array[i]);

    /* Delete value. */
    string_array_del_value(array, "2");
    string_array_del_value(array, "3");
    string_array_del_value(array, "5");

    /* Test values. */
    for (i = 0; expected[i] != NULL; i++) {
        assert_non_null(array[i]);
        assert_string_equal(array[i], expected[i]);
    }
    assert_null(array[i]);

    string_array_free(array);
}

void test_string_array_has_value_safe(void **state)
{
    const char *values[] = {"meeting", "species", "husband", "prosper", NULL};

    assert_true(string_array_has_value_safe((char**)values,
                                            "meeting",
                                            strlen("meeting")));

    assert_false(string_array_has_value_safe((char**)values,
                                             "meeting cool",
                                             strlen("meeting cool")));

    assert_false(string_array_has_value_safe((char**)values,
                                             "meet",
                                             strlen("meet")));

    assert_true(string_array_has_value_safe((char**)values,
                                            "prosper",
                                            strlen("prosper")));

    assert_false(string_array_has_value_safe((char**)values,
                                             "prosperity",
                                             strlen("prosperity")));

    assert_false(string_array_has_value_safe((char**)values,
                                             "prosp",
                                             strlen("prosp")));
}

int main(int argc, const char *argv[])
{

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_string_array_create),
        cmocka_unit_test(test_string_array_del_value__single),
        cmocka_unit_test(test_string_array_del_value__single_repeated),
        cmocka_unit_test(test_string_array_del_value__multiple),
        cmocka_unit_test(test_string_array_del_value__multiple_repeated),
        cmocka_unit_test(test_string_array_has_value_safe)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
