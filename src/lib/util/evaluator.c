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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "evaluator.h"
#include "lib/util/string_array.h"
#include "common/common.h"

enum e_state {
    E_STATE_INVALID = 0,
    E_STATE_BEGIN,
    E_STATE_STRING,
    E_STATE_SUBEXPRESSION,
    E_STATE_OPERATOR,
    E_STATE_UNARY_NOT,
    E_STATE_END,
};

enum e_operator {
    E_OPERATOR_INVALID = 0,
    E_OPERATOR_AND,
    E_OPERATOR_OR,
    E_OPERATOR_NOT
};

struct evaluator {
    const char *expression;
    const char *cursor;
    char *token;
    size_t tokensize;
    int depth;
    enum e_state state;
};

struct e_map {
    char *name;
    enum e_operator value;
};

static struct e_map operators[] = {
    {"and", E_OPERATOR_AND},
    {"or", E_OPERATOR_OR},
    {"not", E_OPERATOR_NOT},
    {NULL, E_OPERATOR_INVALID}
};

static errno_t evaluator_state_machine(struct evaluator *self,
                                       int depth,
                                       const char **features,
                                       bool *_result);

/*
 * This function reads new item/token from expression.
 * It is then stored in self->token. Expected tokens are
 * '(', ')', group of letters (used for operators) or
 * strings (characters enclosed by quotation marks).
 *
 * Strings are copied into self->token with quotation
 * marks and they are used for feature's names.
 *
 */
static errno_t evaluator_next_token(struct evaluator *self)
{
    const char *p;

    while (isblank(*self->cursor)) {
        self->cursor++;
    }

    memset(self->token, 0, self->tokensize);
    switch(tolower(*self->cursor)) {
    case '(':
    case ')':
        strncpy(self->token, self->cursor, 1);
        self->cursor++;
        return EOK;
    case '"':
        p = strchr(&(self->cursor[1]), '"');
        if (p != NULL) {
            strncpy(self->token, self->cursor, p - self->cursor + 1);
            self->cursor = ++p;
            return EOK;
        }
        self->cursor = &self->expression[self->tokensize - 1];
        return EINVAL;
    case '\0':
        return EOK;
    default:
        p = self->cursor;
        while (isalpha(*p)) {
            ++p;
        }
        strncpy(self->token, self->cursor, p - self->cursor);
        self->cursor = p;
        return EOK;
    }
    return EOK;
}

static enum e_operator evaluator_operator(const char *token)
{
    int i;

    if (token == NULL) {
        return E_OPERATOR_INVALID;
    }

    for (i = 0; operators[i].name != NULL; ++i) {
        if (strcasecmp(token, operators[i].name) == 0) {
            return operators[i].value;
        }
    }

    return E_OPERATOR_INVALID;
}

/*
 * This function return the state in which the
 * state should change by evaluating self->token
 *
 * '"feature"' -> E_STATE_STRING -- i. e. feature name
 * '('         -> E_STATE_SUBEXPRESSION -- sub expression
 * ')'         -> E_STATE_END -- end of subexpression
 * 'letters'   -> this should be an operator, find which one
 *                state might be E_STATE_OPERATOR or
 *                E_STATE_UNARY_NOT then
 */
static enum e_state evaluator_token_to_state(const char *token)
{
    enum e_operator operator;

    switch (tolower(token[0])) {
    case '"':
        return E_STATE_STRING;
    case '(':
        return E_STATE_SUBEXPRESSION;
    case ')':
        return E_STATE_END;
    default:
        operator = evaluator_operator(token);
        if (operator != E_OPERATOR_INVALID) {
            if (operator == E_OPERATOR_NOT) {
                return E_STATE_UNARY_NOT;
            }
            return E_STATE_OPERATOR;
        }
    }
    return E_STATE_INVALID;
}

static errno_t evaluator_get_feature(const char *token,
                                     const char **features,
                                     bool *_result)
{
    char *feature_name;
    int len;

    *_result = false;
    if (features == NULL || token == NULL) {
        return EINVAL;
    }

    len = strlen(token) - 2;
    feature_name = strdup(&token[1]);
    if (feature_name == NULL) {
        return ENOMEM;
    }

    feature_name[len] = '\0';
    *_result = string_array_has_value((char **)features, feature_name);
    free(feature_name);
    return EOK;
}

/*
 * Handle state machine in E_STATE_BEGIN state
 */
static errno_t evaluator_handle_begin_state(struct evaluator *self,
                                            int depth,
                                            const char **features,
                                            enum e_state nextstate,
                                            bool *result,
                                            bool *negation,
                                            enum e_operator *operator)
{

    bool sub_result;
    errno_t ret;

    switch (nextstate) {
    case E_STATE_STRING:
        return evaluator_get_feature(self->token, features, result);
    case E_STATE_UNARY_NOT:
        *negation = !*negation;
        return EOK;
    case E_STATE_SUBEXPRESSION:
        ret = evaluator_state_machine(self, depth + 1, features,
                                      &sub_result);
        if (ret != EOK) {
            return ret;
        }
        switch(*operator) {
        case E_OPERATOR_AND:
            *result = *result && (*negation ? !sub_result : sub_result);
            *negation = false;
            break;
        case E_OPERATOR_OR:
            *result = *result || (*negation ? !sub_result : sub_result);
            *negation = false;
            break;
        case E_OPERATOR_INVALID: /* no operator yet */
            *result = (*negation ? !sub_result : sub_result);
            *negation = false;
            break;
        default:
            return EINVAL;
        }
        return EOK;
    default:
        return EINVAL;
    }
}

/*
 * Handle state machine in E_STATE_OPERATOR and E_STATE_UNARY_NOT state
 */
static errno_t evaluator_handle_operator_state(struct evaluator *self,
                                               int depth,
                                               const char **features,
                                               enum e_state nextstate,
                                               bool *result,
                                               bool *negation,
                                               enum e_operator *operator)
{
    bool sub_result;
    errno_t ret;

    switch (nextstate) {
    case E_STATE_STRING:
        switch(*operator) {
        case E_OPERATOR_AND:
            ret = evaluator_get_feature(self->token, features,
                                        &sub_result);
            if (ret != EOK) {
                return ret;
            }
            *result = *result && (*negation ? !sub_result : sub_result);
            *negation = false;
            break;
        case E_OPERATOR_OR:
            ret = evaluator_get_feature(self->token, features,
                                        &sub_result);
            if (ret != EOK) {
                return ret;
            }
            *result = *result || (*negation ? !sub_result : sub_result);
            *negation = false;
            break;
        case E_OPERATOR_INVALID: /* no operator yet */
            ret = evaluator_get_feature(self->token, features,
                                        &sub_result);
            if (ret != EOK) {
                return ret;
            }
            *result = *negation ? !sub_result : sub_result;
            *negation = false;
            break;
        default:
            return EINVAL;
        }
        return EOK;
    case E_STATE_UNARY_NOT:
        *negation = !*negation;
        return EOK;
    case E_STATE_SUBEXPRESSION:
        ret = evaluator_state_machine(self, depth + 1, features,
                                      &sub_result);
        if (ret != EOK) {
            return ret;
        }
        switch(*operator) {
        case E_OPERATOR_AND:
            *result = *result && (*negation ? !sub_result : sub_result);
            *negation = false;
            break;
        case E_OPERATOR_OR:
            *result = *result || (*negation ? !sub_result : sub_result);
            *negation = false;
            break;
        case E_OPERATOR_INVALID: /* no operator yet */
            *result = (*negation ? !sub_result : sub_result);
            *negation = false;
            break;
        default:
            return EINVAL;
        }
        return EOK;
    case E_STATE_END:
        if (depth == 0) {
            return EINVAL; /* too many ) */
        }
        return EOK;
    default:
        return EINVAL;
    }
}

/*
 * Handle state machine in E_STATE_SUBEXPRESSION and E_STATE_STRING state
 */
static errno_t evaluator_handle_expression_state(struct evaluator *self,
                                                 int depth,
                                                 const char **features,
                                                 enum e_state nextstate,
                                                 bool *result,
                                                 bool *negation,
                                                 enum e_operator *operator)
{
    switch (nextstate) {
    case E_STATE_OPERATOR:
        *operator = evaluator_operator(self->token);
        return EOK;
    case E_STATE_END:
        if (depth == 0) {
            return EINVAL; /* too many ) */
        }
        return EOK;
    default:
        return EINVAL;
    }
}

static errno_t evaluator_state_machine(struct evaluator *self,
                                       int depth,
                                       const char **features,
                                       bool *_result)
{
    if (self == NULL || _result == NULL) {
        return EINVAL;
    }

    enum e_state state = E_STATE_BEGIN;
    enum e_state nextstate;
    bool result = false;
    bool negation = false;
    enum e_operator operator = E_OPERATOR_INVALID;
    int ret = EOK;

    do {

        ret = evaluator_next_token(self);
        if (ret != EOK) {
            return ret;
        }
        if (self->token[0] != '\0') {
            nextstate = evaluator_token_to_state(self->token);
            switch(state) {
            case E_STATE_BEGIN:
                ret = evaluator_handle_begin_state(self, depth, features,
                                                   nextstate, &result,
                                                   &negation, &operator);
                if (ret != EOK) {
                    return ret;
                }
                break;
            case E_STATE_UNARY_NOT:
            case E_STATE_OPERATOR:
                ret = evaluator_handle_operator_state(self, depth, features,
                                                      nextstate, &result,
                                                      &negation, &operator);
                if (ret != EOK) {
                    return ret;
                }
                break;
            case E_STATE_SUBEXPRESSION:
            case E_STATE_STRING:
                ret = evaluator_handle_expression_state(self, depth, features,
                                                        nextstate, &result,
                                                        &negation, &operator);
                if (ret != EOK) {
                    return ret;
                }
                if (nextstate == E_STATE_END) {
                    *_result = result;
                    return ret;
                }
                break;
            case E_STATE_END:
                if (depth == 0) {
                    return EINVAL; /* too many ) */
                }
                break;
            case E_STATE_INVALID:
                return EINVAL;
            }
            state = nextstate;
        }
    } while (self->token[0] != '\0');

    if (depth != 0) {
        return EINVAL; /* too many ) */
    }

    if (state == E_STATE_OPERATOR ||
        state == E_STATE_UNARY_NOT ||
        state == E_STATE_BEGIN) {

        return EINVAL; /* expression can't end in this state */
    }

    *_result = result;
    return EOK;
}

/*
 * Set an expression to be evaluated.
 * Returns EOK or an error on failure (ENOMEM).
 */
static errno_t evaluator_set_expression(struct evaluator *self,
                                        const char *expression)
{
    if (self == NULL || expression == NULL) {
        return EINVAL;
    }

    self->expression = expression;
    self->cursor = self->expression;
    self->tokensize = strlen(expression) + 1;
    if (self->token == NULL) {
        free(self->token);
    }
    self->token = malloc(self->tokensize);
    if (self->token == NULL) {
        return ENOMEM;
    }
    return EOK;
}

/*
 * Evaluate expression with set of wariables/features
 * Result is stored in _result variable.
 * Returns EOK or an error on failure.
 */
static errno_t evaluator_evaluate(struct evaluator *self,
                              const char **features,
                              bool *_result)
{
    if (self == NULL || features == NULL || _result == NULL) {
        return EINVAL;
    }

    self->cursor = self->expression;
    return evaluator_state_machine(self, 0, features, _result);
}


errno_t evaluate(const char *expression, const char *features[], bool *_result)
{
    struct evaluator *evaluator;
    int ret;

    evaluator = malloc_zero(struct evaluator);
    if (evaluator == NULL) {
        return ENOMEM;
    }

    ret = evaluator_set_expression(evaluator, expression);
    if (ret != EOK) {
        goto done;
    }

    ret = evaluator_evaluate(evaluator, features, _result);

 done:
    if (evaluator->token) {
        free(evaluator->token);
    }

    free(evaluator);
    return ret;
}
