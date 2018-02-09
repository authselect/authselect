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

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include "lib/authselect_private.h"
#include "lib/authselect_paths.h"
#include "lib/util/string_array.h"
#include "lib/util/template.h"
#include "lib/util/textfile.h"
#include "lib/util/file.h"

static errno_t
check_generated_file(const char *path,
                     const char *expected_content,
                     bool *_is_valid)
{
    char *content;
    errno_t ret;
    bool bret;
    bool cmp;

    INFO("Reading file [%s]", path);

    ret = textfile_read(path, AUTHSELECT_FILE_SIZE_LIMIT, &content);
    if (ret == ENOENT) {
        ERROR("[%s] does not exist!", path);
        *_is_valid = false;
        return EOK;
    } else if (ret == EACCES) {
        ERROR("Unable to read [%s] [%d]: %s", path, ret, strerror(ret));
        *_is_valid = false;
        return EOK;
    } else if (ret != EOK) {
        return ret;
    }

    if (expected_content == NULL) {
        expected_content = "";
    }

    cmp = template_validate_written_content(content, expected_content);
    free(content);
    if (!cmp) {
        ERROR("[%s] has unexpected content!", path);
        *_is_valid = false;
        return EOK;
    }

    INFO("Validating file [%s]", path);

    ret = file_is_regular(path, AUTHSELECT_UID, AUTHSELECT_GID,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, &bret);
    if (ret != EOK) {
        ERROR("Unable to check file [%s] mode [%d]: %s",
              path, ret, strerror(ret));
        return ret;
    }

    *_is_valid = bret;

    return EOK;
}

static errno_t
check_generated_files(struct authselect_files *files,
                      bool *_is_valid)
{
    struct authselect_generated generated[] = GENERATED_FILES(files);
    bool is_valid_result = true;
    bool is_valid = false;
    errno_t ret;
    int i;

    for (i = 0; generated[i].path != NULL; i++) {
        ret = check_generated_file(generated[i].path, generated[i].content,
                                   &is_valid);
        if (ret != EOK) {
            return ret;
        }

        is_valid_result &= is_valid;
        if (!is_valid) {
            WARN("File [%s] was modified outside authselect!",
                 generated[i].path);
        }
    }

    *_is_valid = is_valid_result;

    return EOK;
}

static errno_t
check_symlinks(bool *_is_valid)
{
    struct authselect_symlink symlinks[] = SYMLINK_FILES;
    bool is_valid_result = true;
    bool is_valid;
    errno_t ret;
    int i;

    for (i = 0; symlinks[i].name != NULL; i++) {
        INFO("Validating link [%s]", symlinks[i].name);

        ret = file_links_to(symlinks[i].name, symlinks[i].dest, &is_valid);
        if (ret != EOK) {
            return ret;
        }

        if (!is_valid) {
            ERROR("[%s] was not created by authselect!", symlinks[i].name);
            is_valid_result &= is_valid;
        }
    }

    *_is_valid = is_valid_result;

    return EOK;
}

static errno_t
check_missing_conf(bool *_is_valid)
{
    struct authselect_generated generated[] = GENERATED_FILES_PATHS;
    struct authselect_symlink symlinks[] = SYMLINK_FILES;
    bool is_valid_result = true;
    bool is_valid_symlink;
    errno_t ret;
    int i;

    /* Check that generated files are missing. */
    for (i = 0; generated[i].path != NULL; i++) {
        ret = file_exists(generated[i].path);
        if (ret == EOK) {
            ERROR("File [%s] is still present", generated[i].path);
            is_valid_result = false;
            continue;
        } else if (ret != ENOENT) {
            ERROR("Error while trying to access file [%s] [%d]: %s",
                  generated[i].path, ret, strerror(ret));
            return ret;
        }
    }

    /* Check that symlinks do not exist or they are not symlinks or
     * do not point to generated files. */
    for (i = 0; symlinks[i].name != NULL; i++) {
        ret = file_exists(symlinks[i].name);
        if (ret == EOK) {
            INFO("Checking that file [%s] is not an authselect symbolic link "
                 "to [%s]", symlinks[i].name, symlinks[i].dest);

            ret = file_does_not_link_to(symlinks[i].name, symlinks[i].dest,
                                        &is_valid_symlink);
            if (ret != EOK) {
                return ret;
            }

            if (!is_valid_symlink) {
                ERROR("Symbolic link [%s] to [%s] still exists!",
                      symlinks[i].name, symlinks[i].dest);
                is_valid_result = false;
            }
        } else if (ret != ENOENT) {
            ERROR("Error while trying to access file [%s] [%d]: %s",
                  symlinks[i].name, ret, strerror(ret));
            return ret;
        }
    }

    *_is_valid = is_valid_result;

    return EOK;
}

static errno_t
check_existing_conf(const char *profile_id,
                    const char **features,
                    bool *_is_valid)
{
    struct authselect_files *files;
    bool is_valid_result = true;
    bool is_valid;
    errno_t ret;

    ret = authselect_cat(profile_id, features, &files);
    if (ret != EOK) {
        ERROR("Unable to load profile [%s] [%d]: %s",
              profile_id, ret, strerror(ret));
        return ret;
    }

    /* Check that generated files exist and have proper content. */
    ret = check_generated_files(files, &is_valid);
    if (ret != EOK) {
        return ret;
    }

    is_valid_result &= is_valid;

    /* Check that symlinks exist and point to generated files. */
    ret = check_symlinks(&is_valid);
    if (ret != EOK) {
        return ret;
    }

    is_valid_result &= is_valid;

    *_is_valid = is_valid_result;

    return EOK;
}

_PUBLIC_ int
authselect_check_conf(bool *_is_valid)
{
    char *profile_id;
    char **features;
    errno_t ret;

    ret = authselect_read_conf(&profile_id, &features);
    if (ret == ENOENT) {
        /* No existing configuration was detected.
         * Check that there are no leftovers. */
        ret = check_missing_conf(_is_valid);
        if (ret != EOK) {
            return ret;
        }

        return ENOENT;
    } if (ret != EOK) {
        return ret;
    }

    /* Some configuration is present. Check that everything is valid. */
    ret = check_existing_conf(profile_id, (const char **)features, _is_valid);

    free(profile_id);
    string_array_free(features);

    return ret;
}

errno_t
authselect_check_symlinks_presence(bool *_exist)
{
    struct authselect_symlink symlinks[] = SYMLINK_FILES;
    bool exist = false;
    errno_t ret;
    int i;

    for (i = 0; symlinks[i].name != NULL; i++) {
        ret = file_exists(symlinks[i].name);
        if (ret == EOK) {
            ERROR("File [%s] exist but it needs to be overwritten!",
                  symlinks[i].name);
            exist = true;
        } else if (ret != ENOENT) {
            ERROR("Error while trying to access file [%s] [%d]: %s",
                  symlinks[i].name, ret, strerror(ret));
            return ret;
        }
    }

    *_exist = exist;

    return EOK;
}
