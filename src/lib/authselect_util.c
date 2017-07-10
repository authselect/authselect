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

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "authselect_util.h"

void free_string_array(char **array)
{
    int i;

    if (array == NULL) {
        return;
    }

    for (i = 0; array[i] != NULL; i++) {
        free(array[i]);
    }

    free(array);
}

static errno_t
read_textfile_internal(FILE *file, const char *filename, char **_content)
{
    ssize_t bytes_read;
    char *buffer;
    size_t len;
    errno_t ret;

    buffer = NULL;
    bytes_read = getdelim(&buffer, &len, '\0', file);
    if (bytes_read == -1) {
        ret = errno;
        goto done;
    }

    *_content = buffer;
    ret = EOK;

done:
    /* File descriptor is closed when file is closed. */
    fclose(file);

    if (ret != EOK) {
        ERROR("Unable to read file '%s' [%d]: %s",
               filename, ret, strerror(ret));
    }

    return ret;
}

errno_t
read_textfile(const char *filepath, char **_content)
{
    FILE *file;

    INFO("Reading file '%s'", filepath);

    file = fopen(filepath, "r");
    if (file == NULL) {
        return errno;
    }

    return read_textfile_internal(file, filepath, _content);
}

errno_t
read_textfile_dirfd(int dirfd, const char *filename, char **_content)
{
    errno_t ret;
    FILE *file;
    int fd;

    INFO("Reading file '%s'", filename);

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

    return read_textfile_internal(file, filename, _content);
}
