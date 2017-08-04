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
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "authselect_private.h"

struct authselect_dir {
    int fd;
    DIR *dirstream;
    char *path;
    char **profiles;
    size_t num_profiles;
};

void
authselect_dir_free(struct authselect_dir *dir)
{
    int i;

    if (dir == NULL) {
        return;
    }

    /* Close directory and its associated file descriptor. */
    if (dir->dirstream != NULL) {
        closedir(dir->dirstream);
    }

    if (dir->path != NULL) {
        free(dir->path);
    }

    if (dir->profiles != NULL) {
        for (i = 0; dir->profiles[i] != NULL; i++) {
            free(dir->profiles[i]);
        }

        free(dir->profiles);
    }

    free(dir);
}

static errno_t
authselect_dir_add_profile(struct authselect_dir *dir,
                           const char *name)
{
    char *profile;
    char **resized;

    if (name == NULL) {
        return EINVAL;
    }

    profile = strdup(name);
    if (profile == NULL) {
        return ENOMEM;
    }

    resized = realloc_array(dir->profiles, char *, dir->num_profiles + 2);
    if (resized == NULL) {
        free(profile);
        return ENOMEM;
    }

    resized[dir->num_profiles] = profile;
    resized[dir->num_profiles + 1] = NULL;

    dir->profiles = resized;
    dir->num_profiles++;

    return EOK;

}

static errno_t
authselect_dir_init(const char *path,
                    struct authselect_dir **_dir)
{
    struct authselect_dir *dir;
    errno_t ret;

    if (path == NULL) {
        return EINVAL;
    }

    dir = malloc_zero(struct authselect_dir);
    if (dir == NULL) {
        return ENOMEM;
    }

    dir->dirstream = NULL;
    dir->fd = -1;

    dir->path = strdup(path);
    if (dir->path == NULL) {
        ret = ENOMEM;
        goto done;
    }

    dir->profiles = malloc_zero_array(char *, 1);
    if (dir->profiles == NULL) {
        ret = ENOMEM;
        goto done;
    }

    *_dir = dir;

    ret = EOK;

done:
    if (ret != EOK) {
        authselect_dir_free(dir);
    }

    return ret;
}

static errno_t
authselect_dir_open(const char *path,
                    struct authselect_dir **_dir)
{
    struct authselect_dir *dir;
    errno_t ret;

    ret = authselect_dir_init(path, &dir);
    if (ret != EOK) {
        return ret;
    }

    dir->dirstream = opendir(path);
    if (dir->dirstream == NULL) {
        ret = errno;
        /* If not found, return empty directory. */
        if (ret == ENOENT) {
            *_dir = dir;
        }
        goto done;
    }

    dir->fd = dirfd(dir->dirstream);
    if (dir->fd == -1) {
        ret = errno;
        goto done;
    }

    *_dir = dir;

    ret = EOK;

done:
    if (ret != EOK && ret != ENOENT) {
        authselect_dir_free(dir);
    }

    return ret;
}

errno_t
authselect_dir_read(const char *dirpath,
                    struct authselect_dir **_dir)
{
    struct authselect_dir *dir = NULL;
    struct dirent *entry;
    struct stat statres;
    errno_t ret;

    INFO("Reading profile directory [%s]", dirpath);

    ret = authselect_dir_open(dirpath, &dir);
    if (ret == ENOENT) {
        /* If not found, return empty directory. */
        WARN("Directory [%s] is missing!", dirpath);
        *_dir = dir;
        return EOK;
    } else if (ret != EOK) {
        ERROR("Unable to open directory [%s] [%d]: %s",
              dirpath, ret, strerror(ret));
        goto done;
    }

    /* Read profiles from directory. */
    errno = 0;
    while ((entry = readdir(dir->dirstream)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        ret = fstatat(dir->fd, entry->d_name, &statres, 0);
        if (ret != 0) {
            ret = errno;
            goto done;
        }

        /* Continue with the next one if it is not a directory. */
        if (!S_ISDIR(statres.st_mode)) {
            WARN("Not a directory: %s", entry->d_name);
            continue;
        }

        /* Otherwise take this as a profile and remember. */
        INFO("Found profile [%s]", entry->d_name);
        ret = authselect_dir_add_profile(dir, entry->d_name);
        if (ret != EOK) {
            goto done;
        }
    }

    if (errno != 0) {
        ret = errno;
        goto done;
    }

    *_dir = dir;

    ret = EOK;

done:
    if (ret != EOK) {
        ERROR("Unable to read directory [%s] [%d]: %s",
              dirpath, ret, strerror(ret));
        authselect_dir_free(dir);
    }

    return ret;
}

char **
authselect_merge_profiles(struct authselect_dir *profile,
                          struct authselect_dir *vendor,
                          struct authselect_dir *custom)
{
    size_t num_profiles = 0;
    size_t i, j, id_index;
    char **ids;

    num_profiles += profile->num_profiles;
    num_profiles += vendor->num_profiles;
    num_profiles += custom->num_profiles;

    ids = malloc_zero_array(char *, num_profiles + 1);
    if (ids == NULL) {
        return NULL;
    }

    id_index = 0;

    /* Add all profiles from the default profile directory. */
    for (i = 0; i < profile->num_profiles; i++) {
        ids[id_index] = strdup(profile->profiles[i]);
        if (ids[id_index] == NULL) {
            goto fail;
        }

        id_index++;
    }

    /* Add only new profiles from the vendor profile directory. */
    for (i = 0; i < vendor->num_profiles; i++) {
        for (j = 0; j < profile->num_profiles; j++) {
            if (strcmp(vendor->profiles[i], profile->profiles[j]) == 0) {
                break;
            }
        }

        if (j != profile->num_profiles) {
            continue;
        }

        ids[id_index] = strdup(vendor->profiles[i]);
        if (ids[id_index] == NULL) {
            goto fail;
        }

        id_index++;
    }

    /* Add all profiles from the custom profile directory,
     * prefix them with custom/. */
    for (i = 0; i < custom->num_profiles; i++) {
        ids[id_index] = authselect_profile_custom_id(custom->profiles[i]);
        if (ids[id_index] == NULL) {
            goto fail;
        }

        id_index++;
    }

    return ids;

fail:
    free(ids);
    return NULL;
}
