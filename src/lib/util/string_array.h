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

#ifndef _STRING_ARRAY_H_
#define _STRING_ARRAY_H_

#include <stddef.h>
#include <stdbool.h>

/**
 * Create NULL-terminated string array to hold @num_items.
 *
 * @param num_items Array capacity not including NULL terminator.
 *
 * @return Array on success, NULL if the allocation fails.
 */
char **
string_array_create(size_t num_items);

/**
 * Resize NULL-terminated string array to hold @num_items. If the new capacity
 * is smaller, the elements that do not fit into the new capacity are freed.
 *
 * If reallocation fails, NULL is returned and the original array is freed.
 *
 * @param num_items New array capacity not including NULL terminator.
 *
 * @return Array on success, NULL if the allocation fails.
 */
char **
string_array_resize(char **array, size_t num_items);

/**
 * Free NULL-terminated string array together with its elements.
 *
 * @param array NULL-terminated array.
 */
void
string_array_free(char **array);

/**
 * Count NULL-terminated string array elements.
 *
 * @param array NULL-terminated array.
 *
 * @return Number of elements stored in the array.
 */
size_t
string_array_count(char **array);

/**
 * Check if a value exist in NULL-terminated array.
 *
 * @param array NULL-terminated array.
 *
 * @return True if the array contains the value and false otherwise.
 */
bool
string_array_has_value(char **array, const char *value);

/**
 * Add value to NULL-terminated string array to its end.
 *
 * If reallocation fails, NULL is returned and the original array is freed.
 *
 * @param array NULL-terminated array.
 * @param value Value to append to the array.
 *
 * @return Array or NULL if reallocation fails.
 */
char **
string_array_add_value(char **array, const char *value);

/**
 * Remove value from NULL-terminated string array.
 *
 * @param array NULL-terminated array.
 * @param value Value to remove from the array.
 *
 * @return Array without the value.
 */
char **
string_array_del_value(char **array, const char *value);

#endif /* _STRING_ARRAY_H_ */
