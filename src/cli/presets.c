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

#include <string.h>
#include <stdlib.h>

#include "common/common.h"
#include "lib/util/config.h"
#include "lib/util/string_array.h"
#include "lib/util/file.h"
#include "lib/constants.h"
#include "cli/presets.h"

static const struct cli_preset presets[] = {
    {"@system-default", "Use default system configuration", AUTHSELECT_DEFAULT_CONFIG_PATH},
    {NULL, NULL, NULL}
};

static bool
cli_preset_is_available(const struct cli_preset *preset)
{
    errno_t ret;

    if (preset == NULL) {
        return false;
    }

    ret = file_exists(preset->config_path);
    return ret == EOK;
}

static errno_t
cli_preset_merge_features(const char **preset_features,
                          const char **cli_features,
                          char ***_merged_features)
{
    char **merged;
    int i;

    /* Create new empty array */
    merged = string_array_create(0);
    if (merged == NULL) {
        return ENOMEM;
    }

    /* Add preset features */
    if (preset_features != NULL) {
        for (i = 0; preset_features[i] != NULL; i++) {
            merged = string_array_add_value(merged, preset_features[i], true);
            if (merged == NULL) {
                return ENOMEM;
            }
        }
    }

    /* Add CLI features (skip duplicates) */
    if (cli_features != NULL) {
        for (i = 0; cli_features[i] != NULL; i++) {
            merged = string_array_add_value(merged, cli_features[i], true);
            if (merged == NULL) {
                return ENOMEM;
            }
        }
    }

    *_merged_features = merged;
    return EOK;
}

bool
cli_preset_is_preset(const char *profile_id)
{
    return profile_id != NULL && profile_id[0] == '@';
}

const struct cli_preset*
cli_preset_find(const char *name)
{
    int i;

    if (name == NULL) {
        return NULL;
    }

    for (i = 0; presets[i].name != NULL; i++) {
        if (strcmp(presets[i].name, name) == 0) {
            return &presets[i];
        }
    }

    return NULL;
}

errno_t
cli_preset_resolve(const char *preset_name,
                   const char **additional_features,
                   char **_profile_id,
                   char ***_features)
{
    const struct cli_preset *preset;
    char **preset_features = NULL;
    char *profile_id;
    errno_t ret;

    preset = cli_preset_find(preset_name);
    if (preset == NULL) {
        ERROR("Unknown preset: %s", preset_name);
        return ENOENT;
    }

    ret = config_read(preset->config_path, &profile_id, &preset_features);
    if (ret != EOK) {
        ERROR("Unable to read preset configuration from %s [%d]: %s",
              preset->config_path, ret, strerror(ret));
        return ret;
    }

    if (_features != NULL) {
        ret = cli_preset_merge_features((const char **)preset_features,
                                         additional_features,
                                         _features);
        if (ret != EOK) {
            goto done;
        }
    }
    *_profile_id = profile_id;

    ret = EOK;

done:
    string_array_free(preset_features);
    if (ret != EOK) {
        free(profile_id);
    }

    return ret;
}

struct cli_preset*
cli_preset_list(void)
{
    struct cli_preset *available;
    int count = 0;
    int i, j;

    /* Count available presets */
    for (i = 0; presets[i].name != NULL; i++) {
        if (cli_preset_is_available(&presets[i])) {
            count++;
        }
    }

    /* Allocate array for available presets */
    available = malloc_zero_array(struct cli_preset, count + 1);
    if (available == NULL) {
        return NULL;
    }

    /* Copy available presets */
    j = 0;
    for (i = 0; presets[i].name != NULL; i++) {
        if (cli_preset_is_available(&presets[i])) {
            available[j++] = presets[i];
        }
    }
    available[j].name = NULL;

    return available;
}

void
cli_preset_list_free(struct cli_preset *list)
{
    free(list);
}

int
cli_preset_max_name_length(void)
{
    struct cli_preset *available;
    size_t max = 0;
    size_t len;
    int i;

    available = cli_preset_list();
    if (available == NULL) {
        return 0;
    }

    for (i = 0; available[i].name != NULL; i++) {
        len = strlen(available[i].name);
        if (max < len) {
            max = len;
        }
    }

    cli_preset_list_free(available);

    return max;
}
