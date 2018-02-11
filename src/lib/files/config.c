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
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "common/common.h"
#include "lib/authselect_private.h"
#include "lib/authselect_paths.h"
#include "files/files.h"
#include "util/string_array.h"
#include "util/textfile.h"
#include "util/string.h"
#include "util/file.h"

static char *
authselect_config_read_profile_id(char **config)
{
    return strdup(config[0]);
}

static char **
authselect_config_read_features(char **config)
{
    char **features;
    int i;


    features = string_array_create(0);
    if (features == NULL) {
        return NULL;
    }

    /* Skip profile name. */
    for (i = 1; config[i] != NULL; i++) {
        features = string_array_add_value(features, config[i]);
        if (features == NULL) {
            return NULL;
        }
    }

    return features;
}

errno_t
authselect_config_read(char **_profile_id,
                       char ***_features)
{
    char *profile_id;
    char **features;
    char *content;
    char **lines;
    errno_t ret;

    ret = textfile_read(PATH_CONFIG_FILE,
                        AUTHSELECT_FILE_SIZE_LIMIT,
                        &content);
    if (ret != EOK) {
        return ret;
    }

    lines = string_explode(content, '\n', STRING_EXPLODE_ALL);
    if (lines == NULL) {
        return ENOMEM;
    }

    if (lines[0] == NULL) {
        ret = ENOENT;
        goto done;
    }

    profile_id = authselect_config_read_profile_id(lines);
    if (profile_id == NULL) {
        ret = ENOMEM;
        goto done;
    }

    features = authselect_config_read_features(lines);
    if (features == NULL) {
        free(profile_id);
        ret = ENOMEM;
        goto done;
    }

    *_profile_id = profile_id;
    *_features = features;

    ret = EOK;

done:
    string_array_free(lines);

    return ret;
}

errno_t
authselect_config_write(const char *profile_id,
                        const char **features)
{
    char *implode;
    char *output;
    errno_t ret;

    implode = string_implode(features, '\n');
    if (implode == NULL) {
        return ENOMEM;
    }

    output = format("%s\n%s", profile_id, implode);
    free(implode);
    if (output == NULL) {
        return ENOMEM;
    }

    ret = textfile_write(PATH_CONFIG_FILE, output, AUTHSELECT_FILE_MODE);
    free(output);

    return ret;
}

bool
authselect_config_locations_writable()
{
    const char *dirs[] = {
        AUTHSELECT_CONFIG_DIR,
        AUTHSELECT_PAM_DIR,
        AUTHSELECT_DCONF_DIR,
        AUTHSELECT_DCONF_DIR "/locks",
        NULL, /* place for nsswitch.conf parent directory */
        NULL
    };
    errno_t ret;
    bool result;
    int i;

    /* We need to special case since nsswitch.conf is a file not a
     * directory. But we want to make sure that its parent directory
     * exists and we can write to it.*/
    dirs[4] = file_get_parent_directory(AUTHSELECT_NSSWITCH_CONF);
    if (dirs[4] == NULL) {
        ERROR("Unable to get path to nsswitch.conf parent directory!");
        return false;
    }

    INFO("Checking if all required directories are writable.");

    result = true;
    for (i = 0; dirs[i] != NULL; i++) {
        ret = file_check_access(dirs[i], W_OK | X_OK);
        if (ret == EOK) {
            continue;
        } else if (ret == ENOENT) {
            ERROR("Directory [%s] does not exist, please create it!", dirs[i]);
        } else if (ret != EOK) {
            ERROR("Unable to access directory [%s] in [WX] mode!", dirs[i]);
        }

        result = false;
    }

    free((char *)dirs[4]);

    return result;
}

bool
authselect_config_validate_existing(const char *profile_id,
                                    const char **features)
{
    struct authselect_files *files;
    bool result = true;
    errno_t ret;

    ret = authselect_cat(profile_id, features, &files);
    if (ret != EOK) {
        ERROR("Unable to load profile [%s] [%d]: %s",
              profile_id, ret, strerror(ret));
        return false;
    }

    /* Check that generated files exist and have proper content. */
    result &= authselect_system_validate(files);

    /* Check that symlinks exist and point to generated files. */
    result &= authselect_symlinks_validate();

    return result;
}

bool
authselect_config_validate_non_existing()
{
    bool result = true;

    result &= authselect_system_validate_missing();
    result &= authselect_symlinks_validate_missing();

    return result;
}
