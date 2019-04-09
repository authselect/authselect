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

static errno_t
authselect_profile_dir_read(const char *path,
                            char ***array,
                            bool is_custom)
{
    char *id;
    char **subdirs;
    errno_t ret;
    int i;

    INFO("Reading profile directory [%s]", path);

    ret = dir_list(path, DIR_LIST_DIRS, &subdirs, NULL);
    if (ret == ENOENT) {
        /* This is just a warning so it should not be treated as error. */
        WARN("Directory [%s] is missing!", path);
        return EOK;
    } else if (ret != EOK) {
        ERROR("Unable to list directory [%s] [%d]: %s",
              path, ret, strerror(ret));
        return ret;
    }

    for (i = 0; subdirs[i] != NULL; i++) {
        if (is_custom) {
            /* Custom profile needs to be prefixed with custom/ */
            id = authselect_profile_custom_id(subdirs[i]);
            if (id == NULL) {
                ret = ENOMEM;
                goto done;
            }

            free(subdirs[i]);
            subdirs[i] = id;

        }

        INFO("Found profile [%s]", subdirs[i]);
    }

    *array = string_array_concat(*array, subdirs, true);
    if (*array == NULL) {
        ret = ENOMEM;
        goto done;
    }

    ret = EOK;

done:
    string_array_free(subdirs);

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
