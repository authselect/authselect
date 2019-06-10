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
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "common/common.h"
#include "lib/util/file.h"

static bool
file_check_type(struct stat *statbuf,
                const char *name,
                mode_t mode)
{
    mode_t exp_type = mode & S_IFMT;
    mode_t real_type;

    if (statbuf == NULL) {
        ERROR("Internal error: stat cannot be NULL!");
        return false;
    }

    real_type = statbuf->st_mode & S_IFMT;
    if (exp_type != (real_type)) {
        switch (exp_type) {
        case S_IFDIR:
            ERROR("[%s] is not a directory!", name);
            break;
        case S_IFREG:
            ERROR("[%s] is not a regular file!", name);
            break;
        case S_IFLNK:
            ERROR("[%s] is not a symbolic link!", name);
            break;
        default:
            ERROR("[%s] has wrong type [%.7o], expected [%.7o]!",
                  name, real_type, exp_type);
            break;
        }

        return false;
    }

    return true;
}

static bool
file_check_mode(struct stat *statbuf,
                const char *name,
                uid_t uid,
                gid_t gid,
                mode_t mode)
{
    mode_t exp_perm = mode & ALLPERMS;
    bool bret;

    bret = file_check_type(statbuf, name, mode);
    if (!bret) {
        return false;
    }

    if (exp_perm != (statbuf->st_mode & ALLPERMS)) {
        ERROR("[%s] has wrong mode [%.4o], expected [%.4o]!",
              name, statbuf->st_mode & ALLPERMS, exp_perm);
        return false;
    }

    if (uid != (uid_t)(-1) && statbuf->st_uid != uid) {
        ERROR("[%s] has wrong owner [%u], expected [%u]!",
              name, statbuf->st_uid, uid);
        return false;
    }

    if (gid != (gid_t)(-1) && statbuf->st_gid != gid) {
        ERROR("[%s] has wrong group [%u], expected [%u]!",
              name, statbuf->st_gid, gid);
        return false;
    }

    return true;
}

static errno_t
file_check_attributes(const char *filepath,
                      uid_t uid,
                      gid_t gid,
                      mode_t mode,
                      bool *_result)
{
    struct stat statbuf;
    errno_t ret;

    ret = lstat(filepath, &statbuf);
    if (ret == -1) {
        ret = errno;
        if (ret == ENOENT) {
            ERROR("[%s] does not exist!", filepath);
            *_result = false;
            return EOK;
        }

        ERROR("Unable to stat [%s] [%d]: %s", filepath, ret, strerror(ret));
        return ret;
    }

    *_result = file_check_mode(&statbuf, filepath, uid, gid, mode);

    return EOK;
}

errno_t
file_is_regular(const char *filepath,
           uid_t uid,
           gid_t gid,
           mode_t access_mode,
           bool *_result)
{
    return file_check_attributes(filepath, uid, gid, S_IFREG | access_mode,
                                 _result);
}

errno_t
file_links_to(const char *linkpath,
              const char *destpath,
              bool *_result)
{
    char linkbuf[PATH_MAX + 1];
    ssize_t len;
    errno_t ret;

    ret = file_check_attributes(linkpath, (uid_t)-1, (gid_t)-1,
                                S_IFLNK | ACCESSPERMS, _result);
    if (ret != EOK || *_result == false) {
        return ret;
    }

    len = readlink(linkpath, linkbuf, PATH_MAX + 1);
    if (len == -1) {
        ret = errno;
        ERROR("Unable to read link destination [%s] [%d]: %s",
              linkpath, ret, strerror(ret));
        return ret;
    }

    if (strncmp(linkbuf, destpath, len) != 0) {
        ERROR("Link [%s] does not point to [%s]", linkpath, destpath);
        *_result = false;
        return EOK;
    }

    *_result = true;
    return EOK;
}

errno_t
file_does_not_link_to(const char *linkpath,
                      const char *destpath,
                      bool *_result)
{
    char linkbuf[PATH_MAX + 1];
    struct stat statbuf;
    ssize_t len;
    errno_t ret;

    ret = lstat(linkpath, &statbuf);
    if (ret == -1) {
        ret = errno;

        if (ret == ENOENT) {
            *_result = true;
            return EOK;
        }

        ERROR("Unable to stat [%s] [%d]: %s", linkpath, ret, strerror(ret));
        return ret;
    }

    if (!S_ISLNK(statbuf.st_mode)) {
        *_result = true;
        return EOK;
    }

    len = readlink(linkpath, linkbuf, PATH_MAX + 1);
    if (len == -1) {
        ret = errno;
        ERROR("Unable to read link destination [%s] [%d]: %s",
              linkpath, ret, strerror(ret));
        return ret;
    }

    if (strncmp(linkbuf, destpath, len) == 0) {
        ERROR("Link [%s] points to [%s]", linkpath, destpath);
        *_result = false;
        return EOK;
    }

    *_result = true;
    return EOK;
}

errno_t
file_check_access(const char *path, int mode)
{
    errno = 0;
    if (access(path, mode) == 0) {
        return EOK;
    }

    /* ENOENT is returned if a file is missing. */
    return errno;
}

errno_t
file_exists(const char *path)
{
    return file_check_access(path, F_OK);
}

const char *
file_get_basename(const char *filepath)
{
    const char *filename;

    if (filepath == NULL) {
        return NULL;
    }

    filename = strrchr(filepath, '/');
    if (filename == NULL) {
        /* There is no slash. */
        return filepath;
    }

    if (filename[0] == '\0' || filename[1] == '\0') {
        /* There is a slash but no file name. */
        return NULL;
    }

    return filename + 1;

}

char *
file_get_parent_directory(const char *filepath)
{
    char *copy = NULL;
    char *out = NULL;
    char *dir;
    errno_t ret;

    if (filepath == NULL) {
        ERROR("Internal error: filepath cannot be NULL!");
        return NULL;
    }

    copy = strdup(filepath);
    if (copy == NULL) {
        ret = ENOMEM;
        goto done;
    }

    /* Function dirname() may modify input argument,
     * it may also return pointer to static memory. */
    dir = dirname(copy);
    if (dir == NULL) {
        ret = ENOTDIR;
        goto done;
    }

    out = strdup(dir);
    if (out == NULL) {
        ret = ENOMEM;
        goto done;
    }

    ret = EOK;

done:
    if (copy != NULL) {
        free(copy);
    }

    if (ret != EOK) {
        ERROR("Unable to get parent directory of [%s] [%d]: %s",
              filepath, ret, strerror(ret));
    }

    return out;
}

errno_t
file_make_path(const char *path, mode_t mode)
{
    char *parent;
    errno_t ret;

    ret = file_exists(path);
    if (ret != ENOENT) {
        return ret;
    }

    parent = file_get_parent_directory(path);
    if (parent != NULL) {
        ret = file_make_path(parent, mode);
        free(parent);
        if (ret != EOK) {
            return ret;
        }
    }

    ret = mkdir(path, mode);
    if (ret != 0) {
        return errno;
    }

    return EOK;

}

errno_t
file_mktmp_for(const char *path, mode_t mode, char **_tmpfile)
{
    mode_t oldmask;
    char *tmpfile;
    errno_t ret;
    int fd;

    oldmask = umask(mode);

    tmpfile = format("%s.XXXXXX", path);
    if (tmpfile == NULL) {
        ret = ENOMEM;
        goto done;
    }

    fd = mkstemp(tmpfile);
    if (fd == -1) {
        ret = errno;
        /* To silence static analyzers that assumes that errno can be 0 here. */
        if (ret == EOK) {
            ret = EINVAL;
        }
        goto done;
    }

    close(fd);

    *_tmpfile = tmpfile;

    ret = EOK;

done:
    if (ret != EOK) {
        free(tmpfile);
    }

    umask(oldmask);

    return ret;
}

static errno_t
file_mktmp_at(const char *path, const char *name, mode_t mode, char **_tmpfile)
{
    char *fullpath;
    errno_t ret;

    fullpath = format("%s/%s", path, name);
    if (fullpath == NULL) {
        return ENOMEM;
    }

    ret = file_mktmp_for(fullpath, mode, _tmpfile);
    free(fullpath);

    return ret;
}

errno_t
file_mktmp_copy(const char *source,
                const char *destdir,
                const char *destname,
                mode_t dir_mode,
                char **_tmpfile)
{
    const char *tmpname;
    char *tmpfile;
    errno_t ret;

    ret = file_make_path(destdir, dir_mode);
    if (ret != EOK) {
        return ret;
    }

    ret = file_mktmp_at(destdir, destname, 0600, &tmpfile);
    if (ret != EOK) {
        return ret;
    }

    tmpname = file_get_basename(tmpfile);
    if (tmpname == NULL) {
        ret = EINVAL;
        goto done;
    }

    ret = file_copy(source, destdir, tmpname, dir_mode);
    if (ret != EOK) {
        goto done;
    }

    *_tmpfile = tmpfile;

    ret = EOK;

done:
    if (ret != EOK) {
        unlink(tmpfile);
        free(tmpfile);
    }

    return ret;
}

errno_t
file_copy(const char *source,
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
        goto done;
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

        bytes_written = fwrite(buf, sizeof(char), bytes_read, fdest);
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
