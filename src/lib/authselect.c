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
#include <stdlib.h>
#include <stdbool.h>

#include "authselect.h"

#include "lib/constants.h"
#include "lib/util/util.h"
#include "lib/files/files.h"
#include "lib/profiles/profiles.h"

static bool
authselect_check_features(const struct authselect_profile *profile,
                          const char **features)
{
    const char *similar;
    bool result = true;
    char **supported;
    int i;

    if (features == NULL) {
        return true;
    }

    supported = authselect_profile_features(profile);
    if (supported == NULL) {
        ERROR("Unable to obtain supported features");
        return false;
    }

    for (i = 0; features[i] != NULL; i++) {
        if (string_array_has_value(supported, features[i])) {
            continue;
        }

        result = false;
        similar = string_array_find_similar(features[i], supported, 5);
        if (similar != NULL) {
            ERROR("Unknown profile feature [%s], did you mean [%s]?",
                  features[i], similar);
        } else {
            ERROR("Unknown profile feature [%s]", features[i]);
        }
    }

    string_array_free(supported);

    return result;
}

_PUBLIC_ void
authselect_set_debug_fn(authselect_debug_fn fn, void *pvt)
{
    set_debug_fn(fn, pvt);
}

_PUBLIC_ int
authselect_activate(const char *profile_id,
                    const char **features,
                    bool force_overwrite)
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

    if (!authselect_check_features(profile, features)) {
        ret = EINVAL;
        goto done;
    }

    if (force_overwrite) {
        INFO("Enforcing activation!");
        ret = authselect_profile_activate(profile, features);
        goto done;
    }

    /* Require force if authselect.conf is missing or invalid but otherwise
     * ignore user changes. */
    ret = authselect_validate_configuration(&is_valid);
    if (ret != EOK) {
        ERROR("%s is missing or unreadable, system was not properly configured "
              "by authselect.", PATH_CONFIG_FILE);
        ERROR("Refusing to activate profile unless overwrite is requested.");
        ret = EEXIST;
        goto done;
    }

    if (!is_valid) {
        ERROR("Changes to the authselect configuration were detected. "
              "These changes will be overwritten. Please call "
              "'authselect opt-out' in order to keep them.");
    }

    ret = authselect_profile_activate(profile, features);

done:
    if (ret != EOK && ret != EEXIST) {
        ERROR("Unable to activate profile [%s] [%d]: %s",
              profile_id, ret, strerror(ret));
    }

    authselect_profile_free(profile);

    return ret;
}

_PUBLIC_ int
authselect_uninstall(void)
{
    errno_t ret;

    INFO("Trying to uninstall authselect configuration");

    ret = authselect_symlinks_uninstall();
    if (ret != EOK) {
        ERROR("Unable to remove symlinks [%d]: %s", ret, strerror(ret));
        return ret;
    }

    INFO("Symbolic links were successfully removed");

    /* Remove files from /etc/authselect */
    ret = authselect_files_uninstall();
    if (ret != EOK) {
        ERROR("Unable to remove authselect configuration [%d]: %s",
              ret, strerror(ret));
        return ret;
    }

    INFO("Authselect configuration was successfully removed");

    return EOK;
}

_PUBLIC_ int
authselect_apply_changes(void)
{
    struct authselect_profile *profile = NULL;
    char **supported = NULL;
    char *profile_id;
    char **features;
    errno_t ret;
    int i;

    ret = authselect_current_configuration(&profile_id, &features);
    if (ret != EOK) {
        return ret;
    }

    ret = authselect_profile(profile_id, &profile);
    if (ret != EOK) {
        ERROR("Unable to find profile [%s] [%d]: %s",
              profile_id, ret, strerror(ret));
        goto done;
    }

    supported = authselect_profile_features(profile);
    if (supported == NULL) {
        ERROR("Unable to obtain supported features");
        ret = ENOMEM;
        goto done;
    }

    for (i = 0; features[i] != NULL; i++) {
        if (string_array_has_value(supported, features[i])) {
            continue;
        }

        WARN("Profile feature [%s] is no longer supported, removing it...",
             features[i]);

        string_array_del_value(features, features[i]);
        i--;
    }

    ret = authselect_activate(profile_id, (const char **)features, false);

done:
    authselect_profile_free(profile);
    string_array_free(supported);
    string_array_free(features);
    free(profile_id);

    return ret;
}

_PUBLIC_ int
authselect_feature_enable(const char *feature)
{
    char *profile_id;
    char **features;
    errno_t ret;

    ret = authselect_current_configuration(&profile_id, &features);
    if (ret != EOK) {
        return ret;
    }

    features = string_array_add_value(features, feature, true);
    if (features == NULL) {
        ret = ENOMEM;
        goto done;
    }

    ret = authselect_activate(profile_id, (const char **)features, false);

done:
    string_array_free(features);
    free(profile_id);

    return ret;
}

_PUBLIC_ int
authselect_feature_disable(const char *feature)
{
    char *profile_id;
    char **features;
    errno_t ret;

    ret = authselect_config_read(&profile_id, &features);
    if (ret != EOK) {
        return ret;
    }

    string_array_del_value(features, feature);

    ret = authselect_activate(profile_id, (const char **)features, false);

    string_array_free(features);
    free(profile_id);

    return ret;
}

_PUBLIC_ int
authselect_feature_enabled(const char *feature)
{
    char *profile_id;
    char **features;
    errno_t ret;
    bool bret;

    ret = authselect_config_read(&profile_id, &features);
    if (ret != EOK) {
        return ret;
    }

    bret = string_array_has_value(features, feature);

    string_array_free(features);
    free(profile_id);

    ret = bret ? EOK : ENOENT;

    return ret;
}

_PUBLIC_ int
authselect_validate_configuration(bool *_is_valid)
{
    char *profile_id;
    char **features;
    errno_t ret;

    ret = authselect_config_read(&profile_id, &features);
    if (ret == ENOENT) {
        *_is_valid = authselect_config_validate_user();

        if (*_is_valid && authselect_config_validate_missing()) {
            return ENOENT;
        }

        return EEXIST;
    } if (ret != EOK) {
        return ret;
    }

    *_is_valid = authselect_config_validate_authselect(profile_id,
                                                       (const char **)features);

    free(profile_id);
    string_array_free(features);

    return EOK;
}

_PUBLIC_ int
authselect_current_configuration(char **_profile_id,
                                 char ***_features)
{
    return authselect_config_read(_profile_id, _features);
}

_PUBLIC_ char **
authselect_list()
{
    char **profiles;
    errno_t ret;

    ret = authselect_profile_list(&profiles);
    if (ret != EOK) {
        return NULL;
    }

    return profiles;
}

_PUBLIC_ void
authselect_array_free(char **array)
{
    string_array_free(array);
}
