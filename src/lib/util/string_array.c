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

#include "config.h"

#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include "common/common.h"
#include "lib/util/string_array.h"
#include "lib/util/string.h"

char **
string_array_create(size_t num_items)
{
    return malloc_zero_array(char *, num_items + 1);
}

char **
string_array_copy(char **array, bool unique)
{
    char **copy;
    size_t len;
    size_t i;

    if (array == NULL) {
        return NULL;
    }

    len = string_array_count(array);

    copy = string_array_create(len);
    if (copy == NULL) {
        return NULL;
    }

    for (i = 0; array[i] != NULL; i++) {
        copy = string_array_add_value(copy, array[i], unique);
        if (copy == NULL) {
            return NULL;
        }
    }

    return copy;
}

char **
string_array_resize(char **array, size_t num_items)
{
    char **reallocated;
    size_t count;
    size_t i;

    count = string_array_count(array);
    if (num_items < count) {
        for (i = num_items; i < count; i++) {
            free(array[i]);
            array[i] = NULL;
        }
    }

    reallocated = realloc_array(array, char *, num_items + 1);
    if (reallocated == NULL) {
        string_array_free(array);
        return NULL;
    }

    reallocated[num_items] = NULL;
    return reallocated;
}

void
string_array_free(char **array)
{
    size_t i;

    if (array == NULL) {
        return;
    }

    for (i = 0; array[i] != NULL; i++) {
        free(array[i]);
    }

    free(array);
}

size_t
string_array_count(char **array)
{
    size_t count;

    for (count = 0; array[count] != NULL; count++) {
        /* no op */
    }

    return count;
}

bool
string_array_has_value_safe(char **array, const char *value, size_t len)
{
    int i;

    if (value == NULL) {
        return false;
    }

    for (i = 0; array[i] != NULL; i++) {
        if (strncmp(value, array[i], len) == 0 && array[i][len] == '\0') {
            return true;
        }
    }

    return false;
}

bool
string_array_has_value(char **array, const char *value)
{
    return string_array_has_value_safe(array, value, strlen(value));
}

char **
string_array_add_value_safe(char **array,
                            const char *value,
                            size_t len,
                            bool unique)
{
    size_t count;

    if (unique && string_array_has_value_safe(array, value, len)) {
        return array;
    }

    count = string_array_count(array);
    array = string_array_resize(array, count + 1);
    if (array == NULL) {
        return NULL;
    }

    array[count] = strndup(value, len);
    if (array[count] == NULL) {
        string_array_free(array);
        return NULL;
    }

    return array;
}

char **
string_array_add_value(char **array, const char *value, bool unique)
{
    return string_array_add_value_safe(array, value, strlen(value), unique);
}

void
string_array_del_value(char **array, const char *value)
{
    size_t count;
    size_t pos;
    size_t i;

    if (array == NULL) {
        return;
    }

    count = string_array_count(array);
    for (i = 0; i < count; i++) {
        if (strcmp(array[i], value) == 0) {
            free(array[i]);
            array[i] = NULL;
        }
    }

    for (i = 0, pos = 0; i < count; i++) {
        if (array[i] != NULL) {
            array[pos] = array[i];
            pos++;
        }
    }

    for (; pos < count; pos++) {
        array[pos] = NULL;
    }

    return;
}

char **
string_array_concat(char **to, char **items, bool unique)
{
    int i;

    if (items == NULL) {
        return to;
    }

    for (i = 0; items[i] != NULL; i++) {
        to = string_array_add_value(to, items[i], unique);
        if (to == NULL) {
            return NULL;
        }
    }

    return to;
}

static int
string_array_sort_callback(const void *a, const void *b)
{
    return strcmp(*(char* const*)a, *(char* const*)b);
}

void
string_array_sort(char **array)
{
    if (array == NULL) {
        return;
    }

    qsort(array, string_array_count(array), sizeof(char *),
          string_array_sort_callback);
}

const char *
string_array_find_similar(const char *value, char **array, int max_distance)
{
    const char *word = NULL;
    int current;
    int best = 0;
    int i;

    for (i = 0; array[i] != NULL; i++) {
        current = string_levenshtein(value, array[i]);
        if (word == NULL) {
            best = current;
            word = array[i];
            continue;
        }

        if (current < best) {
            best = current;
            word = array[i];
        }
    }

    if (word == NULL || best > max_distance) {
        return NULL;
    }

    return word;
}
