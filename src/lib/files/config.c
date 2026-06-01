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
#include "lib/constants.h"
#include "lib/util/util.h"
#include "lib/util/config.h"
#include "lib/files/files.h"

errno_t
authselect_config_read(char **_profile_id,
                       char ***_features)
{
    return config_read(PATH_CONFIG_FILE, _profile_id, _features);
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

    output = format("%s\n%s\n", profile_id, implode);
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
    struct authselect_symlink files[] = {
        {PATH_CONFIG_FILE, NULL, false},
        SYMLINK_FILES
    };
    bool result = true;
    char *dirpath;
    errno_t ret;
    int i;

    INFO("Checking if all required directories are writable.");

    for (i = 0; files[i].name != NULL; i++) {
        dirpath = file_get_parent_directory(files[i].name);
        if (dirpath == NULL) {
            ERROR("Unable to get path to %s parent directory!", files[i].name);
            result = false;
            continue;
        }

        ret = file_check_access(dirpath, W_OK | X_OK);
        if (ret == ENOENT && files[i].create_path) {
            INFO("Creating path [%s]", dirpath);
            ret = file_make_path(dirpath, AUTHSELECT_DIR_MODE);
            if (ret != EOK) {
                result = false;
                ERROR("Unable to create path [%s] [%d]: %s",
                      dirpath, ret, strerror(ret));
            }
        } else if (ret == ENOENT) {
            result = false;
            ERROR("Directory [%s] does not exist, please create it!", dirpath);
        } else if (ret != EOK) {
            result = false;
            ERROR("Unable to access directory [%s] in [WX] mode!", dirpath);
        }

        free(dirpath);
    }

    return result;
}

bool
authselect_config_validate_authselect(const char *profile_id,
                                      const char **features)
{
    struct authselect_files *files;
    bool result = true;
    errno_t ret;

    ret = authselect_files(profile_id, features, &files);
    if (ret != EOK) {
        ERROR("Unable to load profile [%s] [%d]: %s",
              profile_id, ret, strerror(ret));
        return false;
    }

    /* Check that generated files exist and have proper content. */
    result &= authselect_system_validate(files);

    /* Check that symlinks exist and point to generated files. */
    result &= authselect_symlinks_validate();

    authselect_files_free(files);

    return result;
}

bool
authselect_config_validate_user()
{
    bool result = true;

    result &= authselect_system_validate_missing();
    result &= authselect_symlinks_validate_missing();

    return result;
}

bool
authselect_config_validate_missing()
{
    struct authselect_generated generated[] = GENERATED_FILES_PATHS;
    struct authselect_symlink symlinks[] = {SYMLINK_FILES};
    errno_t ret;
    int i;

    ret = file_exists(PATH_CONFIG_FILE);
    if (ret != ENOENT) {
        return false;
    }

    for (i = 0; generated[i].path != NULL; i++) {
        ret = file_exists(generated[i].path);
        if (ret != ENOENT) {
            return false;
        }
    }

    for (i = 0; symlinks[i].name != NULL; i++) {
        ret = file_exists(symlinks[i].name);
        if (ret != ENOENT) {
            return false;
        }
    }

    return true;
}
