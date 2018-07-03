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

#include "common/common.h"
#include "lib/util/file.h"
#include "lib/util/textfile.h"

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

errno_t
textfile_copy(const char *source,
              const char *destdir,
              const char *destname,
              mode_t dir_mode)
{
    struct stat statbuf;
    FILE *fsource = NULL;
    FILE *fdest = NULL;
    size_t bytes_written;
    size_t bytes_read;
    mode_t oldmask;
    char *destpath;
    char buf[32];
    errno_t ret;

    ret = file_make_path(destdir, dir_mode);
    if (ret != EOK) {
        return ret;
    }

    destpath = format("%s/%s", destdir, destname);
    if (destpath == NULL) {
        return ENOMEM;
    }

    /* Temporary umask before we change the owner and permissions. */
    oldmask = umask(0600);

    fsource = fopen(source, "r");
    if (fsource == NULL) {
        ret = errno;
        goto done;
    }

    ret = fstat(fileno(fsource), &statbuf);
    if (ret == -1) {
        ret = errno;
        return ret;
    }

    fdest = fopen(destpath, "w");
    if (fdest == NULL) {
        ret = errno;
        goto done;
    }

    do {
        bytes_read = fread(buf, sizeof(char), sizeof(buf), fsource);
        if (bytes_read != sizeof(buf)) {
            if (ferror(fsource) != 0) {
                ret = EIO;
                goto done;
            }
            /* eof not error */
        }

        bytes_written = fwrite(buf, sizeof(char), sizeof(buf), fdest);
        if (bytes_written != bytes_read) {
            if (ferror(fdest) != 0) {
                ret = EIO;
                goto done;
            }
        }
    } while (!feof(fsource));

    /* Restore original owner and mode.  Errors here are not fatal, since we
     * have the original content already stored and owned by root. */
    ret = chmod(destpath, statbuf.st_mode);
    if (ret != 0) {
        ret = errno;
        WARN("Unable to chmod file [%s] [%d]: %s",
             destpath, ret, strerror(ret));
    }

    ret = chown(destpath, statbuf.st_uid, statbuf.st_gid);
    if (ret != 0) {
        ret = errno;
        WARN("Unable to chown file [%s] [%d]: %s",
             destpath, ret, strerror(ret));
    }

    ret = EOK;

done:
    free(destpath);
    umask(oldmask);
    if (fsource != NULL) {
        fclose(fsource);
    }

    if (fdest != NULL) {
        fclose(fdest);
    }

    return ret;
}
