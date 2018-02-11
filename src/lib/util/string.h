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

#define STRING_EXPLODE_TRIM            0x0001
#define STRING_EXPLODE_SKIP_EMPTY      0x0002
#define STRING_EXPLODE_SKIP_COMMENT    0x0004
#define STRING_EXPLODE_ALL             0xFFFF

/**
 * CHeck if the string is empty.
 *
 * @param str String to test.
 *
 * @return True if @str is NULL or empty, false otherwise.
 */
bool
string_is_empty(const char *str);

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

/**
 * Split the string on each delimiter and convert it into NULL-terminated
 * string array.
 *
 * @param str       String to split.
 * @param delimiter Delimiter.
 * @param flags     Bit mask of flags. See STRING_EXPLODE_* macros.
 *
 * @return String array or NULL if allocation fails.
 */
char **
string_explode(const char *str, char delimiter, unsigned int flags);

/**
 * Concatenates items of NULL-terminated string array with delimiter.
 *
 * @param array     Array to concatenate.
 * @param delimiter Delimiter.
 *
 * @return String or NULL if allocation fails.
 */
char *
string_implode(const char **array, char delimiter);

#endif /* _STRING_H_ */
