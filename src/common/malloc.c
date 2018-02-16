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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void *_malloc_zero(size_t size)
{
    void *ptr;

    ptr = malloc(size);
    if (ptr == NULL) {
        return NULL;
    }

    memset(ptr, 0, size);

    return ptr;
}

void *_realloc_array(void *ptr, size_t elsize, size_t num)
{
    return realloc(ptr, elsize * num);
}
