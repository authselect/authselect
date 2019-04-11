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

#ifndef _SELINUX_H_
#define _SELINUX_H_

#include "common/errno_t.h"

/**
 * Get default security context for @path.
 *
 * @param path Path to the file.
 *
 * @return EOK on success, ENOENT if context was not found, other errno code
 *         is returned on failure.
 */
errno_t
selinux_get_default_context(const char *path);

/**
 * Create temporary file created on @filepath.XXXXXX with security context
 * set to default security context of @filepath.
 *
 * @param filepath File for which a temporary file should be created.
 * @param mode     Temporary file mode.
 * @param _tmpfile Created temporary file.
 *
 * @return EOK on success, other errno code on failure.
 */
errno_t
selinux_mkstemp_for(const char *filepath,
                    mode_t mode,
                    char **_tmpfile);

/**
 * Copy file to destination. Directory is created if it does not exist.
 * The original owner, permissions and selinux context of the source
 * file are kept.
 *
 * @param source       Source file name.
 * @param destdir      Destination directory.
 * @param destname     Destination file name.
 * @param dir_mode     Access mode of destination directory if it is created.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
selinux_file_copy(const char *source,
                  const char *destdir,
                  const char *destname,
                  mode_t dir_mode);

/**
 * Make copy of a file @source and store it in temporary file
 * @destdir/@destname.XXXXXX, keeping its selinux context, owner
 * and permissions.
 *
 * @param source       Source file name.
 * @param destdir      Destination directory.
 * @param destname     Destination file name.
 * @param dir_mode     Access mode of destination directory if it is created.
 * @param _tmpfile     Path to created temporary file.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
selinux_mkstemp_copy(const char *source,
                     const char *destdir,
                     const char *destname,
                     mode_t dir_mode,
                     char **_tmpfile);

struct selinux_safe_copy {
    /* Source file name. */
    const char *source;

    /* Destination file name. */
    const char *destination;
};

/**
 * Copy multiple files to their new destination, keeping their ownership,
 * permissions and selinux context. It will first copy the files into
 * temporary files in their new destination to ensure that the destination
 * is writable and there is enough space to hold the file. Then it will
 * rename it to their desired name, overwriting existing file.
 *
 * @param table        File definitions.
 * @param dir_mode     Access mode of destination directory if it is created.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
selinux_copy_files_safely(struct selinux_safe_copy *table,
                          mode_t dir_mode);

#endif /* _SELINUX_H_ */
