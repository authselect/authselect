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

#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>
#include <stdbool.h>

/**
 * Remove whitespaces from left side of the string and return new location
 * within the same string.
 *
 * @param str String to trim from the left side.
 *
 * @return Pointer to the first non-whitespace character of @str.
 */
const char *
string_trim_left_const(const char *str);

/**
 * Remove whitespaces from left side of the string and return new copy
 * of the string.
 *
 * @param str String to trim from the left side.
 *
 * @return New copy of the string with skipped whitespaces from the left side.
 */
char *
string_trim_left(const char *str);

/**
 * Remove whitespaces from right side of the string and return new copy
 * of the string.
 *
 * @param str String to trim from the right side.
 *
 * @return New copy of the string without whitespaces on the right side.
 */
char *
string_trim_right(const char *str);

/**
 * Remove whitespaces from both left and right side of the string.
 *
 * @param str String to trim from both sides.
 *
 * @return New copy of the string trimmed from both sides.
 */
char *
string_trim(const char *str);

/**
 * Remove whitespaces from both left and right side of the string.
 * If the trimmed string is empty, return NULL rather than an empty string.
 *
 * @param str String to trim from both sides.
 *
 * @return New copy of the string trimmed from both sides.
 */
char *
string_trim_noempty(const char *str);

#endif /* _STRING_H_ */
