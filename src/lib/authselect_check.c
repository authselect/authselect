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

#include "lib/authselect_util.h"
#include "lib/authselect_files.h"
#include "lib/authselect_private.h"

static errno_t
check_generated_file(const char *path,
                     const char *expected_content,
                     bool *_is_valid)
{
    char *content;
    errno_t ret;
    bool bret;
    int cmp;

    ret = read_textfile(path, &content);
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

    cmp = strcmp(content, expected_content);
    free(content);
    if (cmp != 0) {
        ERROR("[%s] has unexpected content!", path);
        *_is_valid = false;
        return EOK;
    }

    ret = check_file(path, AUTHSELECT_UID, AUTHSELECT_GID,
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
    struct {
        const char *path;
        const char *expected;
    } generated[] = {
        {PATH_SYSTEM,      files->systemauth},
        {PATH_PASSWORD,    files->passwordauth},
        {PATH_FINGERPRINT, files->fingerprintauth},
        {PATH_SMARTCARD,   files->smartcardauth},
        {PATH_NSSWITCH,    files->nsswitch},
        {NULL, NULL}
    };
    bool is_valid_result = true;
    bool is_valid = false;
    errno_t ret;
    int i;

    for (i = 0; generated[i].path != NULL; i++) {
        ret = check_generated_file(generated[i].path, generated[i].expected, &is_valid);
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
        ret = check_link(symlinks[i].name, symlinks[i].dest, &is_valid);
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
    const char *generated[] = GENERATED_FILES;
    struct authselect_symlink symlinks[] = SYMLINK_FILES;
    bool is_valid_result = true;
    bool is_valid_symlink;
    errno_t ret;
    int i;

    /* Check that generated files are missing. */
    for (i = 0; generated[i] != NULL; i++) {
        ret = check_exists(generated[i]);
        if (ret == EOK) {
            ERROR("File [%s] is still present", generated[i]);
            is_valid_result = false;
            continue;
        } else if (ret != ENOENT) {
            ERROR("Error while trying to access file [%s] [%d]: %s",
                  generated[i], ret, strerror(ret));
            return ret;
        }
    }

    /* Check that symlinks do not exist or they are not symlinks or
     * do not point to generated files. */
    for (i = 0; symlinks[i].name != NULL; i++) {
        ret = check_exists(symlinks[i].name);
        if (ret == EOK) {
            ret = check_notalink(symlinks[i].name, symlinks[i].dest,
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
                    const char **optional,
                    bool *_is_valid)
{
    struct authselect_profile *profile;
    struct authselect_files *files;
    bool is_valid_result = true;
    bool is_valid;
    errno_t ret;

    profile = authselect_profile(profile_id);
    if (profile == NULL) {
        ERROR("Unable to load profile [%s]!", profile_id);
        return ENOMEM;
    }

    files = authselect_cat(profile_id, optional);
    if (files == NULL) {
        ERROR("Unable to generate profile files [%s]!", profile_id);
        return ENOMEM;
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
    char **optional;
    errno_t ret;

    ret = authselect_read_conf(&profile_id, &optional);
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
    ret = check_existing_conf(profile_id, (const char **)optional, _is_valid);

    free(profile_id);
    free_string_array(optional);

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
        ret = check_exists(symlinks[i].name);
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
