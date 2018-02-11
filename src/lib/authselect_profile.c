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
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "authselect.h"
#include "lib/authselect_paths.h"
#include "lib/authselect_private.h"
#include "lib/util/string.h"
#include "lib/util/textfile.h"
#include "lib/files/files.h"

#define CUSTOM_PROFILE_PREFIX "custom/"

static errno_t
read_profile_meta(struct authselect_profile *profile,
                  int dirfd)
{
    char *readme;
    char *name;
    size_t lineend;
    errno_t ret;

    INFO("Reading file [%s/%s]", profile->path, FILE_README);

    ret = textfile_read_dirfd(dirfd, profile->path, FILE_README,
                              AUTHSELECT_FILE_SIZE_LIMIT, &readme);
    if (ret != EOK) {
        return ret;
    }

    profile->description = readme;

    lineend = strcspn(readme, "\r\n");
    if (lineend <= 0) {
        ERROR("Profile [%s] does not contain a name in [%s]!",
              profile->id, FILE_README);
        return EINVAL;
    }

    name = strndup(readme, lineend);
    if (name == NULL) {
        return ENOMEM;
    }

    profile->name = string_trim_noempty(name);
    free(name);
    if (profile->name == NULL) {
        return EINVAL;
    }

    /* An empty line? */
    if (profile->name == NULL) {
        ERROR("Profile [%s] does not contain a name in [%s]!",
              profile->id, FILE_README);
        return EINVAL;
    }

    return EOK;
}

static errno_t
authselect_profile_init(int profile_fd,
                        const char *profile_id,
                        const char *profile_path,
                        struct authselect_profile **_profile)
{
    struct authselect_profile *profile;
    errno_t ret;

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

    profile->path = strdup(profile_path);
    if (profile->path == NULL) {
        ret = ENOMEM;
        goto done;
    }

    ret = read_profile_meta(profile, profile_fd);
    if (ret != EOK) {
        goto done;
    }

    ret = authselect_system_read_templates(profile->path, profile_fd,
                                           &profile->files);
    if (ret != EOK) {
        goto done;
    }

    *_profile = profile;

    ret = EOK;

done:
    if (ret != EOK) {
        authselect_profile_free(profile);
    }

    return ret;
}

static errno_t
authselect_profile_open(const char *profile_id,
                        const char *dirname,
                        const char *dirpath,
                        int *_dirfd,
                        char **_path)
{
    errno_t ret;
    char *path;
    int fd;

    path = format("%s/%s", dirpath, dirname);
    if (path == NULL) {
        ERROR("Out of memory!");
        return ENOMEM;
    }

    fd = open(path, O_DIRECTORY | O_RDONLY);
    if (fd == -1) {
        ret = errno;
        if (ret != ENOENT) {
            ERROR("Unable to open directory [%s] [%d]: %s",
                   path, ret, strerror(ret));
            return ret;
        }
        free(path);
        return ret;
    }

    if (strcmp(dirpath, DIR_CUSTOM_PROFILES) == 0) {
        INFO("Profile [%s] is a custom profile", profile_id);
    } else if (strcmp(dirpath, DIR_VENDOR_PROFILES) == 0) {
        INFO("Profile [%s] is a vendor profile", profile_id);
    } else {
        INFO("Profile [%s] is a default profile", profile_id);
    }

    *_dirfd = fd;
    *_path = path;

    return EOK;
}

static errno_t
authselect_profile_find(const char *profile_id,
                        int *_dirfd,
                        char **_path)
{
    const char *dirname;
    bool is_custom;
    errno_t ret;
    int i;

    /* Default search order. */
    const char *dirs[] = {
        DIR_VENDOR_PROFILES,
        DIR_DEFAULT_PROFILES,
        NULL
    };

    if (profile_id == NULL) {
        return EINVAL;
    }

    dirname = profile_id;

    /* If it is a custom profile, we want to probe only custom directory. */
    is_custom = authselect_is_custom_profile(profile_id, &dirname);
    if (is_custom) {
        dirs[0] = DIR_CUSTOM_PROFILES;
        dirs[1] = NULL;
    }

    for (i = 0; dirs[i] != NULL; i++) {
        ret = authselect_profile_open(profile_id, dirname, dirs[i],
                                      _dirfd, _path);
        if (ret == EOK) {
            return EOK;
        } else if (ret != ENOENT) {
            return ret;
        }
    }

    return ENOENT;
}

char *
authselect_profile_custom_id(const char *profile_dirname)
{
    return format(CUSTOM_PROFILE_PREFIX "%s", profile_dirname, false);
}

bool
authselect_is_custom_profile(const char *profile_id,
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

    if (_profile_dirname != NULL) {
        *_profile_dirname = profile_id + len;
    }

    return true;
}

_PUBLIC_ int
authselect_profile(const char *profile_id,
                   struct authselect_profile **_profile)
{
    struct authselect_profile *profile = NULL;
    char *profile_path = NULL;
    int profile_fd = -1;
    errno_t ret;

    INFO("Looking up profile [%s]", profile_id);

    ret = authselect_profile_find(profile_id, &profile_fd, &profile_path);
    if (ret == ENOENT) {
        ERROR("Profile [%s] was not found!", profile_id);
        goto done;
    } else if (ret != EOK) {
        ERROR("Unable to find profile [%s] [%d]: %s", ret, strerror(ret));
        goto done;
    }

    INFO("Profile [%s] found at [%s]", profile_id, profile_path);

    ret = authselect_profile_init(profile_fd, profile_id, profile_path,
                                  &profile);
    if (ret != EOK) {
        ERROR("Unable to initialize profile [%s] [%d]: %s",
              profile_id, ret, strerror(ret));
        goto done;
    }

    *_profile = profile;

    ret = EOK;

done:
    if (profile_fd != -1) {
        close(profile_fd);
    }

    if (profile_path != NULL) {
        free(profile_path);
    }

    return ret;
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
