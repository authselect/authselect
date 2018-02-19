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

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "common/common.h"
#include "lib/constants.h"
#include "lib/profiles/profiles.h"
#include "lib/files/files.h"
#include "lib/util/util.h"

static const char **
authselect_profile_locations(const char *id)
{
    static const char *packaged[] = {
        DIR_VENDOR_PROFILES,
        DIR_DEFAULT_PROFILES,
        NULL
    };

    static const char *custom[] = {
        DIR_CUSTOM_PROFILES,
        NULL
    };

    if (authselect_profile_is_custom(id)) {
        return custom;
    }

    return packaged;
}

static errno_t
authselect_profile_open_location(const char *path,
                                 int *_dirfd)
{
    int dirfd;
    errno_t ret;

    dirfd = open(path, O_DIRECTORY | O_RDONLY);
    if (dirfd == -1) {
        /* To silence static analyzers that expect errno == 0. */
        ret = errno == EOK ? EINVAL : errno;
        if (ret == ENOENT) {
            return ENOENT;
        }

        ERROR("Unable to open directory [%s] [%d]: %s",
              path, ret, strerror(ret));
        return ret;
    }

    *_dirfd = dirfd;

    return EOK;
}

static errno_t
authselect_profile_open(const char *id,
                        char **_location,
                        int *_dirfd)
{
    const char **locations;
    const char *name;
    char *location;
    errno_t ret;
    int dirfd;
    int i;

    INFO("Looking up profile [%s]", id);

    locations = authselect_profile_locations(id);

    name = authselect_profile_parse_custom(id);
    name = name == NULL ? id : name;

    for (i = 0; locations[i] != NULL; i++) {
        location = format("%s/%s", locations[i], name);
        if (location == NULL) {
            return ENOMEM;
        }

        ret = authselect_profile_open_location(location, &dirfd);
        if (ret == ENOENT) {
            free(location);
            continue;
        } else if (ret != EOK) {
            free(location);
            return ret;
        }

        if (strcmp(location, DIR_CUSTOM_PROFILES) == 0) {
            INFO("Profile [%s] is a custom profile", id);
        } else if (strcmp(location, DIR_VENDOR_PROFILES) == 0) {
            INFO("Profile [%s] is a vendor profile", id);
        } else {
            INFO("Profile [%s] is a default profile", id);
        }

        INFO("Profile [%s] found at [%s]", id, location);

        *_location = location;
        *_dirfd = dirfd;

        return EOK;
    }

    INFO("Profile [%s] was not found", id);

    return ENOENT;
}

static errno_t
authselect_profile_read_meta(const char *location,
                             int dirfd,
                             char **_name,
                             char **_description)
{
    size_t lineend;
    char *trimmed = NULL;
    char *readme = NULL;
    char *name = NULL;
    errno_t ret;

    INFO("Reading file [%s/%s]", location, FILE_README);

    ret = textfile_read_dirfd(dirfd, location, FILE_README,
                              AUTHSELECT_FILE_SIZE_LIMIT, &readme);
    if (ret != EOK) {
        return ret;
    }

    lineend = strcspn(readme, "\r\n");
    if (lineend <= 0) {
        ERROR("Profile [%s] does not contain a name in [%s]!",
              location, FILE_README);
        ret = EINVAL;
        goto done;
    }

    name = strndup(readme, lineend);
    if (name == NULL) {
        ret = ENOMEM;
        goto done;
    }

    trimmed = string_trim(name);
    if (trimmed == NULL) {
        ret = ENOMEM;
        goto done;
    }

    if (string_is_empty(trimmed)) {
        ERROR("Profile [%s] does not contain a name in [%s]!",
              location, FILE_README);
        ret = EINVAL;
        goto done;
    }

    *_description = readme;
    *_name = trimmed;

    ret = EOK;

done:
    if (name != NULL) {
        free(name);
    }

    if (ret != EOK) {
        free(readme);

        if (trimmed != NULL) {
            free(trimmed);
        }
    }

    return ret;
}

static struct authselect_profile *
authselect_profile_init(const char *id)
{
    struct authselect_profile *profile;

    profile = malloc_zero(struct authselect_profile);
    if (profile == NULL) {
        return NULL;
    }

    profile->id = strdup(id);
    if (profile->id == NULL) {
        free(profile);
        return NULL;
    }

    return profile;
}

errno_t
authselect_profile_read(const char *profile_id,
                        struct authselect_profile **_profile)
{
    struct authselect_profile *profile = NULL;
    char *location;
    int dirfd;
    errno_t ret;

    ret = authselect_profile_open(profile_id, &location, &dirfd);
    if (ret != EOK) {
        return ret;
    }

    profile = authselect_profile_init(profile_id);
    if (profile == NULL) {
        ret = ENOMEM;
        goto done;
    }

    profile->path = location;

    ret = authselect_profile_read_meta(location, dirfd, &profile->name,
                                       &profile->description);
    if (ret != EOK) {
        goto done;
    }

    ret = authselect_system_read_templates(location, dirfd, &profile->files);
    if (ret != EOK) {
        goto done;
    }

    *_profile = profile;

    ret = EOK;

done:
    close(dirfd);

    if (ret != EOK) {
        ERROR("Unable to find profile [%s] [%d]: %s",
              profile_id, ret, strerror(ret));
        authselect_profile_free(profile);
    }

    return ret;
}
