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
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib/authselect_util.h"
#include "lib/authselect_files.h"
#include "lib/authselect_private.h"

static errno_t
file_exists(const char *filename)
{
    errno = 0;
    if (access(filename, F_OK) == 0) {
        return EOK;
    }

    /* ENOENT is returned if a file is missing. */
    return errno;
}

static bool
check_file_mode(struct stat *statbuf,
                const char *filename,
                uid_t uid,
                gid_t gid,
                mode_t mode)
{
    if (statbuf == NULL) {
        ERROR("Internal error: stat cannot be NULL!");
        return false;
    }

    INFO("Checking file %s", filename);

    /* File is a different type. */
    if ((mode & S_IFMT) != (statbuf->st_mode & S_IFMT)) {
        ERROR("File [%s] has wrong type!", filename);
        return false;
    }

    if ((mode & ALLPERMS) != (statbuf->st_mode & ALLPERMS)) {
        ERROR("File [%s] has wrong mode [%.7o], expected [%.7o]!",
              filename, statbuf->st_mode & ALLPERMS, mode & ALLPERMS);
        return false;
    }

    if (uid != (uid_t)(-1) && statbuf->st_uid != uid) {
        ERROR("File [%s] has wrong owner [%u], expected [%u]!",
              filename, statbuf->st_uid, uid);
        return false;
    }

    if (gid != (gid_t)(-1) && statbuf->st_gid != gid) {
        ERROR("File [%s] has wrong group [%u], expected [%u]!",
              filename, statbuf->st_gid, gid);
        return false;
    }

    return true;
}

static errno_t
check_generated_file(const char *path,
                     const char *expected_content,
                     bool *_is_valid)
{
    struct stat statbuf;
    char *content;
    errno_t ret;
    bool bret;
    int cmp;

    ret = read_textfile(path, &content);
    if (ret == ENOENT) {
        ERROR("File [%s] does not exist!", path);
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
        ERROR("File [%s] has unexpected content!");
        *_is_valid = false;
        return EOK;
    }

    ret = lstat(path, &statbuf);
    if (ret == -1) {
        ret = errno;
        ERROR("Unable to stat [%s] [%d]: %s", path, ret, strerror(ret));
        return ret;
    }

    bret = check_file_mode(&statbuf, path, AUTHSELECT_UID, AUTHSELECT_GID,
                           S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (!bret) {
        *_is_valid = false;
        return EOK;
    }

    *_is_valid = true;

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
check_symlink_exist(const char *name,
                    const char *dest,
                    bool *_is_valid)
{
    char linkbuf[PATH_MAX + 1];
    struct stat statbuf;
    ssize_t len;
    errno_t ret;

    ret = lstat(name, &statbuf);
    if (ret == -1) {
        ret = errno;
        ERROR("Unable to stat [%s] [%d]: %s", name, ret, strerror(ret));
        return ret;
    }

    if (!S_ISLNK(statbuf.st_mode)) {
        ERROR("File [%s] is not a symbolic link to [%s]", name, dest);
        *_is_valid = false;
        return EOK;
    }

    len = readlink(name, linkbuf, PATH_MAX + 1);
    if (len == -1) {
        ret = errno;
        ERROR("Unable to readlink [%s] [%d]: %s", name, ret, strerror(ret));
        return ret;
    }

    if (strncmp(linkbuf, dest, len) != 0) {
        ERROR("Symbolic link [%s] does not point to [%s]", name, dest);
        *_is_valid = false;
        return EOK;
    }

    *_is_valid = true;

    return EOK;
}

static errno_t
check_symlink_missing(const char *name,
                      const char *dest,
                      bool *_is_valid)
{
    char linkbuf[PATH_MAX + 1];
    struct stat statbuf;
    ssize_t len;
    errno_t ret;

    ret = lstat(name, &statbuf);
    if (ret == -1) {
        ret = errno;
        ERROR("Unable to stat [%s] [%d]: %s", name, ret, strerror(ret));
        return ret;
    }

    if (!S_ISLNK(statbuf.st_mode)) {
        *_is_valid = true;
        return EOK;
    }

    len = readlink(name, linkbuf, PATH_MAX + 1);
    if (len == -1) {
        ret = errno;
        ERROR("Unable to readlink [%s] [%d]: %s", name, ret, strerror(ret));
        return ret;
    }

    if (strncmp(linkbuf, dest, len) != 0) {
        *_is_valid = true;
        return EOK;
    }

    *_is_valid = false;

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
        ret = check_symlink_exist(symlinks[i].name, symlinks[i].dest, &is_valid);
        if (ret != EOK) {
            return ret;
        }

        if (!is_valid) {
            WARN("Symbolic link [%s] was not created by authselect!",
                 symlinks[i].name);
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
        ret = file_exists(generated[i]);
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

    /* Check that pam.d symlink does not exist or it is not a symlink or
     * it does not point to generated files. */
    for (i = 0; symlinks[i].name != NULL; i++) {
        ret = file_exists(symlinks[i].name);
        if (ret == EOK) {
            ret = check_symlink_missing(symlinks[i].name, symlinks[i].dest,
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
