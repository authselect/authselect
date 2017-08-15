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

#ifndef _AUTHSELECT_UTIL_H_
#define _AUTHSELECT_UTIL_H_

#include <stddef.h>

#include "authselect.h"
#include "common/common.h"

/* Functions marked with this macro are exported to the library consumer. */
#define _PUBLIC_

void free_string_array(char **array);

/* Remove whitespace characters from front and back of a string. */
errno_t
trimline(const char *str, char **_trimmed);

/* Read file contents. */
errno_t
read_textfile(const char *filepath, char **_content);

errno_t
read_textfile_dirfd(int dirfd,
                    const char *dirpath,
                    const char *filename,
                    char **_content);

#endif /* _AUTHSELECT_UTIL_H_ */
