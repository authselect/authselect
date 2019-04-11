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

#ifndef _FILE_H_
#define _FILE_H_

#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "common/errno_t.h"

/**
 * Check that a file @filepath is a regular file.
 *
 * It also checks that the file is owned by given user and group and it has
 * given access mode. Result of these tests is stored in @_result.
 *
 * @param filepath       Path to the file that should be checked.
 * @param uid            Expected UID. Use (uid_t)-1 to allow any user.
 * @param gid            Expected GID. Use (gid_t)-1 to allow any group.
 * @param access_mode    Expected access mode.
 *
 * @param _result        True if all conditions were met, false otherwise.
 *
 * @return On success, EOK is returned and @_result contains result of the
 *         tests. Other errno code is returned on failure.
 */
errno_t
file_is_regular(const char *filepath,
                uid_t uid,
                gid_t gid,
                mode_t access_mode,
                bool *_result);

/**
 * Check that @linkpath is a link to @destpath.
 *
 * The result is stored in @_result.
 *
 * @param linkpath       Path to the file that should be checked.
 * @param destpath       Expected link destination.
 *
 * @param _result        True if all conditions were met, false otherwise.
 *
 * @return On success, EOK is returned and @_result contains result of the
 *         tests. Other errno code is returned on failure.
 */
errno_t
file_links_to(const char *linkpath,
              const char *destpath,
              bool *_result);

/**
 * Check that @linkpath is not a link or it does not link to @destpath.
 *
 * The result is stored in @_result.
 *
 * @param linkpath       Path to the file that should be checked.
 * @param destpath       Destination to which the link must not point.
 *
 * @param _result        True if all conditions were met, false otherwise.
 *
 * @return On success, EOK is returned and @_result contains result of the
 *         tests. Other errno code is returned on failure.
 */
errno_t
file_does_not_link_to(const char *linkpath,
                      const char *destpath,
                      bool *_result);

/**
 * Check file access mode.
 *
 * @param path           Path to the file that should be checked.
 * @param mode           Desired mode. See access(): R_OK, W_OK, X_OK, F_OK
 *
 * @return EOK if the file can be access with desired mode. ENOENT if the file
 *         does not exist. Other errno code on error.
 */
errno_t
file_check_access(const char *path, int mode);

/**
 * Check if file exists.
 *
 * @param path           Path to the file that should be checked.
 *
 * @return EOK if the file exists. ENOENT if the file does not exist.
 *         Other errno code on error.
 */
errno_t
file_exists(const char *path);

/**
 * Return file basename from path.
 *
 * @param filepath Path to the file.
 *
 * @return Basename or NULL if none is present in @filepath.
 */
const char *
file_get_basename(const char *filepath);

/**
 * Return parent directory of a file.
 *
 * @param path           Path to the file.
 *
 * @return Parent directory of the file or NULL on error.
 */
char *
file_get_parent_directory(const char *filepath);

/**
 * Create all directories in a path. Path must end with a directory not a file.
 *
 * @param path           Path to the file whose directories should be created.
 * @param mode           Directory mode.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
file_make_path(const char *path, mode_t mode);

/**
 * Make temporary file for @path so it can be first written and then safely
 * renamed to @path.
 *
 * @param path           Path to the file whose directories should be created.
 * @param mode           Temporary file mode.
 * @param _tmpfile       Path to created temporary file.
 */
errno_t
file_mktmp_for(const char *path, mode_t mode, char **_tmpfile);

/**
 * Copy file to destination. Directory is created if it does not exist.
 * The original owner and permissions of the source file are kept.
 *
 * @param source       Source file name.
 * @param destdir      Destination directory.
 * @param destname     Destination file name.
 * @param dir_mode     Access mode of destination directory if it is created.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
file_copy(const char *source,
          const char *destdir,
          const char *destname,
          mode_t dir_mode);

#endif /* _FILE_H_ */
