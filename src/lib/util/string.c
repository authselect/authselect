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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "common/common.h"
#include "util/string.h"
#include "util/string_array.h"

bool
string_is_empty(const char *str)
{
    return str == NULL || str[0] == '\0';
}

const char *
string_trim_left_const(const char *str)
{
    if (str == NULL) {
        return NULL;
    }

    while (isspace(*str)) {
        str++;
    }

    return str;
}

char *
string_trim_left(const char *str)
{
    str = string_trim_left_const(str);
    if (str == NULL) {
        return NULL;
    }

    return strdup(str);
}

char *
string_trim_right(const char *str)
{
    const char *end;

    if (str == NULL) {
        return NULL;
    }

    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) {
        end--;
    }

    return strndup(str, end - str + 1);
}

char *
string_trim(const char *str)
{
    return string_trim_right(string_trim_left_const(str));
}

char *
string_trim_noempty(const char *str)
{
    char *trimmed = string_trim(str);

    if (trimmed != NULL && trimmed[0] == '\0') {
        free(trimmed);
        return NULL;
    }

    return trimmed;
}

static errno_t
string_explode_get_token(const char *str,
                         size_t len,
                         unsigned int flags,
                         char **_token)
{
    char *token;
    char *tmp;

    token = strndup(str, len);
    if (token == NULL) {
        return ENOMEM;
    }

    if (flags & STRING_EXPLODE_TRIM) {
        tmp = string_trim(token);
        if (tmp == NULL) {
            free(token);
            return ENOMEM;
        }

        token = tmp;
    }

    if (flags & STRING_EXPLODE_SKIP_EMPTY && string_is_empty(token)) {
        *_token = NULL;
        free(token);
        return EOK;
    }

    if (flags & STRING_EXPLODE_SKIP_COMMENT && token[0] == '\0') {
        *_token = NULL;
        free(token);
        return EOK;
    }

    *_token = token;

    return EOK;
}

static char **
string_explode_add_value(char **array,
                         const char *value,
                         size_t len,
                         unsigned int flags)
{
    char *token;
    errno_t ret;

    ret = string_explode_get_token(value, len, flags, &token);
    if (ret != EOK) {
        string_array_free(array);
        return NULL;
    }

    if (token == NULL) {
        return array;
    }

    array = string_array_add_value(array, token, false);
    free(token);

    return array;
}

char **
string_explode(const char *str, char delimiter, unsigned int flags)
{
    const char *remainder;
    const char *pos;
    char **array;
    size_t len;

    array = string_array_create(1);
    if (array == NULL) {
        return NULL;
    }

    remainder = str;
    while ((pos = strchr(remainder, delimiter)) != NULL) {
        len = pos - remainder;
        array = string_explode_add_value(array, remainder, len, flags);
        if (array == NULL) {
            return NULL;
        }

        remainder = pos + 1;
    }

    if (remainder != str && *(remainder - 1) == delimiter) {

    }

    if (string_is_empty(remainder)) {
        /* Add empty line if string end with delimiter. */
        if (remainder != str && *(remainder - 1) == delimiter
                && !(flags & STRING_EXPLODE_SKIP_EMPTY)) {
            return string_array_add_value(array, "", false);
        }

        return array;
    }

    return string_explode_add_value(array, remainder, strlen(remainder), flags);
}

char *
string_implode(const char **array, char delimiter)
{
    const char delimiter_str[] = {delimiter, '\0'};
    size_t len = 0;
    char *tmp;
    char *str;
    int i;

    for (i = 0; array[i] != NULL; i++) {
        len += strlen(array[i]) + 1;
    }

    if (len == 0) {
        return strdup("");
    }

    str = malloc_zero_array(char, len + 1);
    if (str == NULL) {
        return NULL;
    }

    tmp = str;
    for (i = 0; array[i + 1] != NULL; i++) {
        strcat(tmp, array[i]);
        strcat(tmp, delimiter_str);
        tmp += strlen(array[i]) + 1;
    }

    strcat(tmp, array[i]);

    return str;
}
