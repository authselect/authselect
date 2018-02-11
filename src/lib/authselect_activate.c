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
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "authselect.h"
#include "lib/authselect_private.h"
#include "lib/authselect_paths.h"
#include "lib/util/string_array.h"
#include "lib/util/template.h"
#include "lib/util/file.h"
#include "lib/files/files.h"

static errno_t
authselect_activate_profile(struct authselect_profile *profile,
                            const char **features)
{
    errno_t ret;

    ret = authselect_system_write(features, &profile->files);
    if (ret != EOK) {
        ERROR("Unable to write generated system files [%d]: %s",
              ret, strerror(ret));
        goto done;
    }

    ret = authselect_config_write(profile->id, features);
    if (ret != EOK) {
        ERROR("Unable to write configuration [%d]: %s", ret, strerror(ret));
        goto done;
    }

    ret = authselect_symlinks_write();
    if (ret != EOK) {
        ERROR("Unable to create symbolic links [%d]: %s", ret, strerror(ret));
        goto done;
    }

done:
    return ret;
}

_PUBLIC_ int
authselect_activate(const char *profile_id,
                    const char **features,
                    bool force_override)
{
    struct authselect_profile *profile;
    bool is_valid;
    errno_t ret;

    INFO("Trying to activate profile [%s]", profile_id);

    ret = authselect_profile(profile_id, &profile);
    if (ret != EOK) {
        ERROR("Unable to find profile [%s] [%d]: %s",
              profile_id, ret, strerror(ret));
        return ret;
    }

    /* Check that all directories are writable. */
    is_valid = authselect_config_locations_writable();
    if (is_valid == false) {
        ERROR("Some directories are not accessible by authselect!");
        ret = EPERM;
        goto done;
    }

    if (force_override) {
        INFO("Enforcing activation!");
        ret = authselect_activate_profile(profile, features);
        goto done;
    }

    /* First, check that current configuration is valid. */
    ret = authselect_check_conf(&is_valid);
    if (ret != EOK && ret != ENOENT) {
        ERROR("Unable to check configuration [%d]: %s", ret, strerror(ret));
        goto done;
    }

    if (!is_valid) {
        ERROR("Unexpected changes to the configuration were detected.");
        ERROR("Refusing to activate profile unless those changes are removed "
              "or overwrite is requested.");
        ret = AUTHSELECT_ERR_FORCE_REQUIRED;
        goto done;
    }

    /* If no configuration is present, check for existing files. */
    if (ret == ENOENT) {
        if (!authselect_symlinks_location_available()) {
            ERROR("File that needs to be overwritten was found");
            ERROR("Refusing to activate profile unless this file is removed "
                  "or overwrite is requested.");
            ret = AUTHSELECT_ERR_FORCE_REQUIRED;
            goto done;
        }
    }

    ret = authselect_activate_profile(profile, features);

done:
    if (ret != EOK && ret != AUTHSELECT_ERR_FORCE_REQUIRED) {
        ERROR("Unable to activate profile [%s] [%d]: %s",
              profile_id, ret, strerror(ret));
    }

    authselect_profile_free(profile);

    return ret;
}

_PUBLIC_ int
authselect_feature_enable(const char *feature)
{
    char *profile_id;
    char **features;
    errno_t ret;

    ret = authselect_current(&profile_id, &features);
    if (ret != EOK) {
        return ret;
    }

    features = string_array_add_value(features, feature);
    if (features == NULL) {
        ret = ENOMEM;
        goto done;
    }

    ret = authselect_activate(profile_id, (const char **)features, false);

done:
    authselect_features_free(features);
    free(profile_id);

    return ret;
}

_PUBLIC_ int
authselect_feature_disable(const char *feature)
{
    char *profile_id;
    char **features;
    errno_t ret;

    ret = authselect_current(&profile_id, &features);
    if (ret != EOK) {
        return ret;
    }

    features = string_array_del_value(features, feature);
    if (features == NULL) {
        ret = ENOMEM;
        goto done;
    }

    ret = authselect_activate(profile_id, (const char **)features, false);

done:
    authselect_features_free(features);
    free(profile_id);

    return ret;
}
