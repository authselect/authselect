/*
    Authors:
        Pavel Březina <pbrezina@redhat.com>

    Copyright (C) 2017 Red Hat

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

#include <stdarg.h>
#include <stdio.h>

char *
vaformat(const char *fmt, va_list in_va)
{
    char *str = NULL;
    va_list va;
    int ret;

    va_copy(va, in_va);
    ret = vasprintf(&str, fmt, va);
    va_end(va);

    if (ret == -1) {
       return NULL;
    }

    return str;
}

char *
format(const char *fmt, ...)
{
    char *str;
    va_list va;

    va_start(va, fmt);
    str = vaformat(fmt, va);
    va_end(va);

    return str;
}
