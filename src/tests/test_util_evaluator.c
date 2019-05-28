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
#include "lib/util/evaluator.c"

#define evaluator_free(ptr) do { \
        if (ptr) { \
            if ((ptr)->token) {   \
                free((ptr)->token);             \
            } \
            free(ptr); \
        } \
    } while (0)

void test_evaluator_token_reader(void **state)
{
    const char *expression = "not \"w1\"   and\t( \"w2\" or \"w3\" ) or(\"w\"))";
    const char *tokens[] = {
        "not",
        "\"w1\"",
        "and",
        "(",
        "\"w2\"",
        "or",
        "\"w3\"",
        ")",
        "or",
        "(",
        "\"w\"",
        ")",
        ")",
    };
    int i;
    int ret;

    struct evaluator *evaluator = malloc_zero(struct evaluator);
    assert_non_null(evaluator);
    i = evaluator_set_expression(evaluator, expression);
    assert_int_equal(i, 0);

    for(i=0; i<sizeof(tokens)/sizeof(char *); i++) {
        ret = evaluator_next_token(evaluator);
        assert_int_equal(ret, EOK);
        assert_string_equal(evaluator->token, tokens[i]);
    }

    ret = evaluator_next_token(evaluator);
    assert_int_equal(ret, EOK);
    assert_int_equal(evaluator->token[0], '\0');
    evaluator_free(evaluator);
}

void test_evaluator_token_reader_invalid_token(void **state)
{
    const char *expression = "not \"w1 and"; /* missing right " */
    const char *tokens[] = {
        "not",
    };
    int i;
    int ret;

    struct evaluator *evaluator = malloc_zero(struct evaluator);
    assert_non_null(evaluator);
    ret = evaluator_set_expression(evaluator, expression);
    assert_int_equal(ret, EOK);

    for(i=0; i<sizeof(tokens)/sizeof(char *); i++) {
        ret = evaluator_next_token(evaluator);
        assert_int_equal(ret, EOK);
        assert_string_equal(evaluator->token, tokens[i]);
    }

    ret = evaluator_next_token(evaluator);
    assert_int_not_equal(ret, EOK);
    assert_int_equal(evaluator->token[0], '\0');
    evaluator_free(evaluator);
}


void test_evaluator_simple_expressions(void **state)
{
    const char *variables[] = {"w1", "w2", NULL};
    bool result;
    int ret;

    ret = evaluate("\"w1\"", variables, &result);
    assert_int_equal(ret, 0);
    assert_true(result);

    ret = evaluate("\"w3\"", variables, &result);
    assert_int_equal(ret, 0);
    assert_false(result);

    ret = evaluate("\"w1\" or \"w3\"", variables, &result);
    assert_int_equal(ret, 0);
    assert_true(result);

    ret = evaluate("\"w4\" or \"w3\"", variables, &result);
    assert_int_equal(ret, 0);
    assert_false(result);

    ret = evaluate("\"w1\" and \"w2\"", variables, &result);
    assert_int_equal(ret, 0);
    assert_true(result);

    ret = evaluate("\"w1\" and \"w3\"", variables, &result);
    assert_int_equal(ret, 0);
    assert_false(result);

    ret = evaluate("\"w1\" and not \"w3\"", variables, &result);
    assert_int_equal(ret, 0);
    assert_true(result);
}

void test_evaluator_parentheses(void **state)
{
    const char *variables[] = {"w1", "w2", NULL};
    bool result;
    int ret;

    ret = evaluate("(\"w1\")", variables, &result);
    assert_int_equal(ret, 0);
    assert_true(result);

    ret = evaluate("(\"w1\" or \"w5\") and \"w2\"", variables, &result);
    assert_int_equal(ret, 0);
    assert_true(result);

    ret = evaluate("\"w2\" and (\"w1\" or \"w5\")", variables, &result);
    assert_int_equal(ret, 0);
    assert_true(result);

    ret = evaluate("\"w1\" and (\"w5\" or (\"w1\" and \"w2\"))", variables, &result);
    assert_int_equal(ret, 0);
    assert_true(result);

    ret = evaluate("\"w1\" and (\"w5\" and (\"w1\" and \"w2\"))", variables, &result);
    assert_int_equal(ret, 0);
    assert_false(result);
}

void test_evaluator_broken_expressions(void **state)
{
    const char *variables[] = {"w1", "w2", NULL};
    bool result;
    int ret;

    ret = evaluate("()", variables, &result);
    assert_int_not_equal(ret, 0);

    ret = evaluate("and \"w1\"", variables, &result);
    assert_int_not_equal(ret, 0);

    ret = evaluate("", variables, &result);
    assert_int_not_equal(ret, 0);

    ret = evaluate("\"w1\" and or \"w2\"", variables, &result);
    assert_int_not_equal(ret, 0);

    ret = evaluate("\"w1\" and (\"w1\"", variables, &result);
    assert_int_not_equal(ret, 0);

    ret = evaluate("\"w1\" (\"w1\")", variables, &result);
    assert_int_not_equal(ret, 0);

    ret = evaluate("\"w1\") and \"w3\"", variables, &result);
    assert_int_not_equal(ret, 0);

    ret = evaluate("\"w1\" and \"w3\" not", variables, &result);
    assert_int_not_equal(ret, 0);

    ret = evaluate("\"w1\" or", variables, &result);
    assert_int_not_equal(ret, 0);

    ret = evaluate("\"w1\" or \"w3", variables, &result);
    assert_int_not_equal(ret, 0);
}


int main(int argc, const char *argv[])
{

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_evaluator_token_reader),
        cmocka_unit_test(test_evaluator_token_reader_invalid_token),
        cmocka_unit_test(test_evaluator_simple_expressions),
        cmocka_unit_test(test_evaluator_parentheses),
        cmocka_unit_test(test_evaluator_broken_expressions),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
