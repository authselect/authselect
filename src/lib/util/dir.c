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

#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common/common.h"
#include "lib/util/dir.h"
#include "lib/util/string_array.h"

static bool
is_dot_dir(const char *name)
{
    return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

static bool
is_directory(struct dirent *entry, int dirfd)
{
    struct stat statres;
    errno_t ret;

#ifdef _DIRENT_HAVE_D_TYPE
    if (entry->d_type == DT_DIR) {
        return true;
    } else if (entry->d_type != DT_UNKNOWN) {
        return false;
    }
#endif

    /* We must use stat() if d_type is not available or it couldn't determine
     * the type (which may happen on some filesystems). */

    ret = fstatat(dirfd, entry->d_name, &statres, 0);
    if (ret != 0) {
        ret = errno;
        ERROR("Unable to stat directory [%d]: %s", ret, strerror(ret));
        return false;
    }

    if (S_ISDIR(statres.st_mode)) {
        return true;
    }

    return false;
}

static errno_t
dir_open(const char *path,
         DIR **_dirstream,
         int *_descriptor)
{
    DIR *dirstream;
    int descriptor;
    errno_t ret;

    if (path == NULL) {
        return EINVAL;
    }

    dirstream = opendir(path);
    if (dirstream == NULL) {
        /* To silence static analyzers that expect errno == 0. */
        return errno == EOK ? EINVAL : errno;
    }

    /* Descriptor is closed when closedir() is called. */
    descriptor = dirfd(dirstream);
    if (descriptor == -1) {
        ret = errno;
        closedir(dirstream);
        return ret;
    }

    *_dirstream = dirstream;
    *_descriptor = descriptor;

    return EOK;
}

errno_t
dir_list(const char *path,
         uint32_t flags,
         char ***_items,
         int *_dirfd)
{
    struct dirent *entry;
    DIR *dirstream;
    char *fullpath;
    char **items;
    int dirfd;
    int dupfd;
    errno_t ret;

    ret = dir_open(path, &dirstream, &dirfd);
    if (ret != EOK) {
        return ret;
    }

    items = string_array_create(1);
    if (items == NULL) {
        ret = ENOMEM;
        goto done;
    }

    errno = 0;
    while ((entry = readdir(dirstream)) != NULL) {
        if (is_dot_dir(entry->d_name)) {
            continue;
        }

        if (is_directory(entry, dirfd)) {
            if (!(flags & DIR_LIST_DIRS)) {
                continue;
            }
        } else {
            if (!(flags & DIR_LIST_FILES)) {
                continue;
            }
        }

        if (flags & DIR_LIST_FULL_PATH) {
            fullpath = format("%s/%s", path, entry->d_name);
            if (fullpath == NULL) {
                ret = ENOMEM;
                goto done;
            }

            items = string_array_add_value(items, fullpath, true);
            free(fullpath);
        } else {
            items = string_array_add_value(items, entry->d_name, true);
        }

        if (items == NULL) {
            ret = ENOMEM;
            goto done;
        }
    }

    if (_dirfd != NULL) {
        dupfd = dup(dirfd);
        if (dupfd == -1) {
            ret = errno;
            goto done;
        }

        *_dirfd = dupfd;
    }

    *_items = items;

    ret = EOK;

done:
    if (ret == EOK && _dirfd != NULL)  {
        closedir(dirstream);
    }

    return ret;
}
