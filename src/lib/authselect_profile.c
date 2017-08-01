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

#include "config.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "authselect.h"
#include "authselect_files.h"
#include "authselect_private.h"

#define CUSTOM_PROFILE_PREFIX "custom/"

static char *
concatenate(const char *part1, const char *part2, bool add_path_delim)
{
    const char *fmt;
    size_t bufsize;
    char *buf;
    int ret;

    if (part1 == NULL || part2 == NULL) {
        return NULL;
    }

    bufsize = strlen(part1) + strlen(part2) + 1;
    if (add_path_delim) {
        bufsize++;
    }

    buf = malloc_zero_array(char, bufsize);
    if (buf == NULL) {
        return NULL;
    }

    fmt = add_path_delim ? "%s/%s" : "%s%s";
    ret = snprintf(buf, bufsize + 1, fmt, part1, part2);
    if (ret >= bufsize || ret < 0) {
        free(buf);
        return NULL;
    }

    return buf;
}

static bool
is_custom_profile(const char *profile_id,
                  const char **_profile_dirname)
{
    size_t len;

    if (profile_id == NULL) {
        return false;
    }

    len = strlen(CUSTOM_PROFILE_PREFIX);

    if (strncmp(CUSTOM_PROFILE_PREFIX, profile_id, len) != 0) {
        return false;
    }

    *_profile_dirname = profile_id + len;
    return true;
}

static errno_t
read_profile_files(struct authselect_profile *profile,
                   int dirfd)
{
    errno_t ret;
    int i;
    struct {
        const char *filename;
        char **_storage;
    } files[] = {
        {FILE_SYSTEM,      &profile->files.systemauth},
        {FILE_PASSWORD,    &profile->files.passwordauth},
        {FILE_SMARTCARD,   &profile->files.smartcardauth},
        {FILE_FINGERPRINT, &profile->files.fingerprintauth},
        {FILE_NSSWITCH,    &profile->files.nsswitch},
        {NULL, NULL},
    };

    for (i = 0; files[i].filename != NULL; i++) {
        ret = read_textfile_dirfd(dirfd, files[i].filename, files[i]._storage);
        if (ret == ENOENT) {
            *files[i]._storage = NULL;
        } else if (ret != EOK) {
            return ret;
        }
    }

    return EOK;
}

static errno_t
read_profile_meta(struct authselect_profile *profile,
                  int dirfd)
{
    char *readme;
    size_t lineend;
    errno_t ret;

    ret = read_textfile_dirfd(dirfd, FILE_README, &readme);
    if (ret != EOK) {
        return ret;
    }

    profile->description = readme;

    lineend = strcspn(readme, "\r\n");
    if (lineend <= 0) {
        ERROR("Profile README does not contain a name!\n");
        return EINVAL;
    }

    profile->name = strndup(readme, lineend);
    if (profile->name == NULL) {
        return ENOMEM;
    }

    return EOK;
}

static errno_t
authselect_profile_init(struct authselect_dir *dir,
                        const char *profile_id,
                        const char *profile_dirname,
                        struct authselect_profile **_profile)
{
    struct authselect_profile *profile;
    int dirfd;
    errno_t ret;

    dirfd = openat(dir->fd, profile_dirname, O_DIRECTORY | O_RDONLY);
    if (dirfd == -1) {
        return errno;
    }

    profile = malloc_zero(struct authselect_profile);
    if (profile == NULL) {
        ret = ENOMEM;
        goto done;
    }

    profile->id = strdup(profile_id);
    if (profile->id == NULL) {
        ret = ENOMEM;
        goto done;
    }

    profile->path = concatenate(dir->path, profile_dirname, true);
    if (profile->path == NULL) {
        ret = ENOMEM;
        goto done;
    }

    ret = read_profile_meta(profile, dirfd);
    if (ret != EOK) {
        goto done;
    }

    ret = read_profile_files(profile, dirfd);
    if (ret != EOK) {
        goto done;
    }

    *_profile = profile;

    ret = EOK;

done:
    close(dirfd);

    if (ret != EOK) {
        authselect_profile_free(profile);
    }

    return ret;
}

static errno_t
authselect_profile_find(const char *profile_id,
                        const char **_profile_dirname,
                        struct authselect_dir **_dir)
{
    const char *profile_dirname = profile_id;
    struct authselect_dir *dir;
    bool is_custom;
    errno_t ret;
    int i;

    if (profile_id == NULL) {
        return EINVAL;
    }

    /* If it is a custom profile, we can just return the directory. */
    is_custom = is_custom_profile(profile_id, &profile_dirname);
    if (is_custom) {
        INFO("Profile '%s' is a custom profile", profile_id);
        ret = authselect_dir_read(DIR_CUSTOM_PROFILES, &dir);
        goto done;
    }

    /* Check if it a vendor profile. */
    ret = authselect_dir_read(DIR_VENDOR_PROFILES, &dir);
    if (ret != EOK) {
        goto done;
    }

    for (i = 0; dir->profiles[i] != NULL; i++) {
        if (strcmp(dir->profiles[i], profile_id) == 0) {
            /* It is found in vendor directory, return it. */
            INFO("Profile '%s' is a vendor profile", profile_id);
            goto done;
        }
    }

    /* Otherwise it is a default profile. */
    authselect_dir_free(dir);
    INFO("Profile '%s' is a default profile", profile_id);
    ret = authselect_dir_read(DIR_DEFAULT_PROFILES, &dir);

done:
    if (ret != EOK) {
        return ret;
    }

    *_profile_dirname = profile_dirname;
    *_dir = dir;

    return EOK;
}

char *
authselect_profile_custom_id(const char *profile_dirname)
{
    return concatenate(CUSTOM_PROFILE_PREFIX, profile_dirname, false);
}

_PUBLIC_ struct authselect_profile *
authselect_profile(const char *profile_id)
{
    struct authselect_dir *dir = NULL;
    struct authselect_profile *profile = NULL;
    const char *profile_dirname;
    errno_t ret;

    INFO("Looking up profile '%s'", profile_id);

    ret = authselect_profile_find(profile_id, &profile_dirname, &dir);
    if (ret == ENOENT) {
        WARN("Profile '%s' does not exist!", profile_id);
        goto done;
    } else if (ret != EOK) {
        ERROR("Unable to find profile '%s' [%d]: %s", ret, strerror(ret));
        goto done;
    }

    ret = authselect_profile_init(dir, profile_id, profile_dirname, &profile);
    if (ret != EOK) {
        ERROR("Unable to initialize profile '%s' [%d]: %s",
              profile_id, ret, strerror(ret));
        goto done;
    }

    INFO("Profile '%s' found at '%s'", profile_id, dir->path);

    ret = EOK;

done:
    authselect_dir_free(dir);

    if (ret != EOK) {
        return NULL;
    }

    return profile;
}

_PUBLIC_ const char *
authselect_profile_id(const struct authselect_profile *profile)
{
    if (profile == NULL) {
        return NULL;
    }

    return profile->id;
}

_PUBLIC_ const char *
authselect_profile_name(const struct authselect_profile *profile)
{
    if (profile == NULL) {
        return NULL;
    }

    return profile->name;
}

_PUBLIC_ const char *
authselect_profile_path(const struct authselect_profile *profile)
{
    if (profile == NULL) {
        return NULL;
    }

    return profile->path;
}

_PUBLIC_ const char *
authselect_profile_description(const struct authselect_profile *profile)
{
    if (profile == NULL) {
        return NULL;
    }

    return profile->description;
}

_PUBLIC_ void
authselect_profile_free(struct authselect_profile *profile)
{
    if (profile == NULL) {
        return;
    }

    if (profile->id != NULL) {
        free(profile->id);
    }

    if (profile->path != NULL) {
        free(profile->path);
    }

    if (profile->description != NULL) {
        free(profile->description);
    }

    authselect_files_free_content(&profile->files);

    free(profile);

    return;
}
