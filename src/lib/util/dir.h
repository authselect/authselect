/*
    Authors:
        Pavel Březina <pbrezina@redhat.com>

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

#ifndef _DIR_H_
#define _DIR_H_

#include "common/errno_t.h"

/* List files. */
#define DIR_LIST_FILES     0x0001

/* List directories. */
#define DIR_LIST_DIRS      0x0002

/* Use fully qualified paths in the output. */
#define DIR_LIST_FULL_PATH 0x0004

/**
 * List items in a directory.
 *
 * @param path   Directory to list.
 * @param flags  See DIR_LIST_* macros.
 * @param _items NULL-terminated string array that hold the directory items.
 * @param _dirfd If not NULL an open file descriptor of this directory is
 *               stored here.
 *
 * @return EOK on success, ENOENT if the directory was not found, other
 * errno code on failure.
 */
errno_t
dir_list(const char *path,
         uint32_t flags,
         char ***_items,
         int *_dirfd);

#endif /* _DIR_H_ */
