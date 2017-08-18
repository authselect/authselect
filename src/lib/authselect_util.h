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
#include <sys/types.h>

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

/**
 * Check regular file mode.
 *
 * @param filepath    Path to the file.
 * @param uid         Expected owner uid (-1 means do not check).
 * @param gid         Expected group gid (-1 means do not check).
 * @param permissions Expected permissions.
 * @param _result     True if the check pass, false otherwise.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
check_file(const char *filepath,
           uid_t uid,
           gid_t gid,
           mode_t permissions,
           bool *_result);

/**
 * Check link.
 *
 * @param linkpath    Path to the link name.
 * @param destpath    Path to the link destination.
 * @param _result     True if the check pass, false otherwise.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
check_link(const char *linkpath,
           const char *destpath,
           bool *_result);

/**
 * Check that file is not a link or it does not point to @destpath.
 *
 * @param linkpath    Path to the file name.
 * @param destpath    Path to the unwanted link destination.
 * @param _result     True if the check pass, false otherwise.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
check_notalink(const char *linkpath,
               const char *destpath,
               bool *_result);

/**
 * Check access mode.
 *
 * @param path    Path to test.
 * @param mode    Desirect access mode, see @access.
 *
 * @return EOK on success, ENOENT if the path does not exist,
 * other errno code on error.
 */
errno_t
check_access(const char *path, int mode);

/**
 * Check that a path exist.
 *
 * @param path    Path to test.
 *
 * @return EOK if it does exist, ENOENT if it does not,
 * other errno code on error.
 */
errno_t
check_exists(const char *path);

#endif /* _AUTHSELECT_UTIL_H_ */
