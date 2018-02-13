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

#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "common/common.h"
#include "lib/constants.h"
#include "lib/profiles/profiles.h"
#include "lib/util/util.h"

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
authselect_profile_dir_open(const char *path,
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
        return errno;
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

static errno_t
authselect_profile_dir_add(char *name,
                           bool is_custom,
                           char ***array)
{
    char *id = NULL;

    id = !is_custom ? name : authselect_profile_custom_id(name);
    if (id == NULL) {
        return ENOMEM;
    }

    if (string_array_has_value(*array, id)) {
        return EOK;
    }

    *array = string_array_add_value(*array, id);

    if (is_custom) {
        free(id);
    }

    if (*array == NULL) {
        return ENOMEM;
    }

    return EOK;
}

static errno_t
authselect_profile_dir_read(const char *path,
                            char ***array,
                            bool is_custom)
{
    struct dirent *entry;
    DIR *dirstream;
    int dirfd;
    errno_t ret;

    INFO("Reading profile directory [%s]", path);

    ret = authselect_profile_dir_open(path, &dirstream, &dirfd);
    if (ret == ENOENT) {
        /* This is just a warning so it should not be treated as error. */
        WARN("Directory [%s] is missing!", path);
        return EOK;
    } else if (ret != EOK) {
        ERROR("Unable to open directory [%s] [%d]: %s",
              path, ret, strerror(ret));
        return ret;
    }

    errno = 0;
    while ((entry = readdir(dirstream)) != NULL) {
        if (is_dot_dir(entry->d_name)) {
            continue;
        }

        if (!is_directory(entry, dirfd)) {
            WARN("Not a directory: %s", entry->d_name);
            continue;
        }

        INFO("Found profile [%s]", entry->d_name);

        ret = authselect_profile_dir_add(entry->d_name, is_custom, array);
        if (ret != EOK) {
            goto done;
        }
    }

    ret = EOK;

done:
    closedir(dirstream);

    return ret;
}

static int
authselect_profile_list_sort(const void *a, const void *b)
{
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;

    if (str_a == NULL) {
        return 1;
    }

    if (str_b == NULL) {
        return -1;
    }

    bool is_custom_a = authselect_profile_is_custom(str_a);
    bool is_custom_b = authselect_profile_is_custom(str_b);

    /* Custom profiles go last, otherwise we sort alphabetically. */
    if (is_custom_a && !is_custom_b) {
        return 1;
    }

    if (!is_custom_a && is_custom_b) {
        return -1;
    }

    return strcmp(str_a, str_b);
}


errno_t
authselect_profile_list(char ***_profiles)
{
    char **profiles;
    errno_t ret;

    profiles = string_array_create(1);
    if (profiles == NULL) {
        return ENOMEM;
    }

    ret = authselect_profile_dir_read(DIR_DEFAULT_PROFILES, &profiles, false);
    if (ret != EOK) {
        goto done;
    }

    ret = authselect_profile_dir_read(DIR_VENDOR_PROFILES, &profiles, false);
    if (ret != EOK) {
        goto done;
    }

    ret = authselect_profile_dir_read(DIR_CUSTOM_PROFILES, &profiles, true);
    if (ret != EOK) {
        goto done;
    }

    qsort(profiles, string_array_count(profiles), sizeof(char *),
          authselect_profile_list_sort);

    *_profiles = profiles;

    ret = EOK;

done:
    if (ret != EOK) {
        ERROR("Unable to list profiles [%d]: %s", ret, strerror(ret));
        string_array_free(profiles);
    }

    return ret;
}
