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

#ifndef _TEXTFILE_H_
#define _TEXTFILE_H_

#include <sys/stat.h>

#include "common/errno_t.h"

/**
 * Read file contents.
 *
 * If the file is larger then @limit_KiB an error is returned.
 *
 * @param filepath     Path to the file.
 * @param limit_KiB    File size limit in KiB.
 *
 * @param _content     Output variable where file content is stored.
 *
 * @return On success, file content is stored in @_content and EOK is returned.
 *         If the file is larger then @limit_KiB ERANGE is returned. Other
 *         errno code is returned on other error.
 */
errno_t
textfile_read(const char *filepath,
              unsigned int limit_KiB,
              char **_content);

/**
 * Read file contents of a file in directory opened at @dirfd descriptor.
 *
 * If the file is larger then @limit_KiB an error is returned.
 *
 * @param dirfd        File descriptor of an opened directory.
 * @param dirpath      Path to the directory.
 * @param filename     Name of the file.
 * @param limit_KiB    File size limit in KiB.
 *
 * @param _content     Output variable where file content is stored.
 *
 * @return On success, file content is stored in @_content and EOK is returned.
 *         If the file is larger then @limit_KiB ERANGE is returned. Other
 *         errno code is returned on other error.
 */
errno_t
textfile_read_dirfd(int dirfd,
                    const char *dirpath,
                    const char *filename,
                    unsigned int limit_KiB,
                    char **_content);

/**
 * Write file contents to a file. If the file does not exist, it is created,
 * otherwise its content is truncated. The file mode is set to @mode.
 *
 *
 * @param filepath     Path to the file.
 * @param content      Content to write.
 * @param mode         Mode to create the file with.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
textfile_write(const char *filepath,
               const char *content,
               mode_t mode);

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
textfile_copy(const char *source,
              const char *destdir,
              const char *destname,
              mode_t dir_mode);

#endif /* _TEXTFILE_H_ */
