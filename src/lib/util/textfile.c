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

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "util/textfile.h"
#include "common/common.h"

static errno_t
textfile_read_content(FILE *file,
                      const char *filename,
                      unsigned int limit_KiB,
                      char **_content)
{
    size_t read_bytes;
    char *buffer;
    long filelen;
    errno_t ret;

    ret = fseek(file, 0, SEEK_END);
    if (ret != 0) {
        ret = errno;
        goto done;
    }

    filelen = ftell(file);
    if (filelen == -1) {
        ret = errno;
        goto done;
    }

    if (filelen > limit_KiB * 1024) {
        ERROR("File [%s] is bigger than %uKiB!", filename, limit_KiB);
        ret = ERANGE;
        goto done;
    }

    rewind(file);

    buffer = malloc_zero_array(char, filelen + 1);
    if (buffer == NULL) {
        ret = ENOMEM;
        goto done;
    }

    read_bytes = fread(buffer, sizeof(char), filelen, file);
    if (read_bytes != filelen) {
        free(buffer);
        ret = EIO;
        goto done;
    }

    *_content = buffer;

    ret = EOK;

done:
    /* File descriptor is closed when file is closed. */
    fclose(file);

    if (ret != EOK) {
        ERROR("Unable to read file [%s] [%d]: %s",
              filename, ret, strerror(ret));
    }

    return ret;
}

errno_t
textfile_read(const char *filepath,
              unsigned int limit_KiB,
              char **_content)
{
    FILE *file;

    file = fopen(filepath, "r");
    if (file == NULL) {
        return errno;
    }

    return textfile_read_content(file, filepath, limit_KiB, _content);
}

errno_t
textfile_read_dirfd(int dirfd,
                    const char *dirpath,
                    const char *filename,
                    unsigned int limit_KiB,
                    char **_content)
{
    errno_t ret;
    FILE *file;
    int fd;

    fd = openat(dirfd, filename, O_RDONLY);
    if (fd == -1) {
        return errno;
    }

    file = fdopen(fd, "r");
    if (file == NULL) {
        ret = errno;
        close(fd);
        return ret;
    }

    return textfile_read_content(file, filename, limit_KiB, _content);
}

errno_t
textfile_write(const char *filepath,
               const char *content,
               mode_t mode)
{
    FILE *file;
    mode_t oldmask;
    size_t written;
    size_t len;
    errno_t ret;

    if (filepath == NULL || filepath[0] == '\0') {
        return EINVAL;
    }

    /* Create an empty file if no content is given. */
    if (content == NULL) {
        content = "";
    }

    oldmask = umask(mode);

    file = fopen(filepath, "w");
    if (file == NULL) {
        ret = errno;
        ERROR("Unable to open file [%s] [%d]: %s",
              filepath, ret, strerror(ret));
        goto done;
    }

    len = strlen(content);
    written = fwrite(content, sizeof(char), len, file);
    if (written != len) {
        ret = errno;
        ERROR("Unable to write data [%s] [%d]: %s",
              filepath, ret, strerror(ret));
        goto done;
    }

    ret = chmod(filepath, mode);
    if (ret != 0) {
        ret = errno;
        ERROR("Unable to chmod file [%s] [%d]: %s",
              filepath, ret, strerror(ret));
        goto done;
    }

    ret = EOK;

done:
    umask(oldmask);
    if (file != NULL) {
        fclose(file);
    }

    if (ret != EOK) {
        unlink(filepath);
    }

    return ret;
}
