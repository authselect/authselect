/*
    Authors:
        Pavel Březina <pbrezina@redhat.com>

    Copyright (C) 2024 Red Hat

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
#include <string.h>
#include <stdlib.h>

#include "common/common.h"
#include "lib/constants.h"
#include "lib/util/util.h"
#include "lib/util/config.h"

static char *
config_read_profile_id(char **config)
{
    return strdup(config[0]);
}

static char **
config_read_features(char **config)
{
    char **features;
    int i;

    features = string_array_create(0);
    if (features == NULL) {
        return NULL;
    }

    /* Skip profile name. */
    for (i = 1; config[i] != NULL; i++) {
        features = string_array_add_value(features, config[i], true);
        if (features == NULL) {
            return NULL;
        }
    }

    return features;
}

errno_t
config_read(const char *path,
            char **_profile_id,
            char ***_features)
{
    char *profile_id;
    char **features;
    char *content;
    char **lines;
    errno_t ret;

    ret = textfile_read(path, AUTHSELECT_FILE_SIZE_LIMIT, &content);
    if (ret != EOK) {
        return ret;
    }

    lines = string_explode(content, '\n', STRING_EXPLODE_ALL);
    free(content);
    if (lines == NULL) {
        return ENOMEM;
    }

    if (lines[0] == NULL) {
        ret = ENOENT;
        goto done;
    }

    profile_id = config_read_profile_id(lines);
    if (profile_id == NULL) {
        ret = ENOMEM;
        goto done;
    }

    if (_features != NULL) {
        features = config_read_features(lines);
        if (features == NULL) {
            free(profile_id);
            ret = ENOMEM;
            goto done;
        }

        *_features = features;
    }

    *_profile_id = profile_id;

    ret = EOK;

done:
    string_array_free(lines);

    return ret;
}
