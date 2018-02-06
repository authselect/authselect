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

#include <stddef.h>

#include "authselect.h"

#include "lib/authselect_private.h"
#include "lib/authselect_paths.h"
#include "lib/authselect_util.h"

_PUBLIC_ void
authselect_set_debug_fn(authselect_debug_fn fn, void *pvt)
{
    set_debug_fn(fn, pvt);
}

_PUBLIC_ int
authselect_current(char **_profile_id,
                   char ***_features)
{
    return authselect_read_conf(_profile_id, _features);
}

_PUBLIC_ void
authselect_features_free(char **features)
{
    free_string_array(features);
}

_PUBLIC_ char **
authselect_list()
{
    struct authselect_dir *profile = NULL;
    struct authselect_dir *vendor = NULL;
    struct authselect_dir *custom = NULL;
    char **profiles = NULL;
    errno_t ret;

    ret = authselect_dir_read(DIR_DEFAULT_PROFILES, &profile);
    if (ret != EOK) {
        goto done;
    }

    ret = authselect_dir_read(DIR_VENDOR_PROFILES, &vendor);
    if (ret != EOK) {
        goto done;
    }

    ret = authselect_dir_read(DIR_CUSTOM_PROFILES, &custom);
    if (ret != EOK) {
        goto done;
    }

    profiles = authselect_merge_profiles(profile, vendor, custom);

done:
    authselect_dir_free(profile);
    authselect_dir_free(vendor);
    authselect_dir_free(custom);

    return profiles;
}

_PUBLIC_ void
authselect_list_free(char **profile_ids)
{
    free_string_array(profile_ids);
}

_PUBLIC_ int
authselect_cat(const char *profile_id,
               const char **features,
               struct authselect_files **_files)
{
    struct authselect_profile *profile;
    struct authselect_files *files;
    errno_t ret;

    ret = authselect_profile(profile_id, &profile);
    if (ret != EOK) {
        return ret;
    }

    ret = authselect_files_generate(profile, features, &files);
    authselect_profile_free(profile);
    if (ret != EOK) {
        return ret;
    }

    *_files = files;

    return EOK;
}
