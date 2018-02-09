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

#include "util/string.h"

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
