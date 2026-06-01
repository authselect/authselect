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

#ifndef _PRESETS_H_
#define _PRESETS_H_

#include <stdbool.h>
#include "common/common.h"

/**
 * Preset profile information.
 */
struct cli_preset {
    const char *name;           /* Preset name (e.g., "@system-default") */
    const char *description;    /* Human-readable description */
    const char *config_path;    /* Path to configuration file */
};

/**
 * Check if a profile identifier is a preset.
 *
 * @param profile_id    Profile identifier to check.
 * @return true if it's a preset (starts with '@'), false otherwise.
 */
bool
cli_preset_is_preset(const char *profile_id);

/**
 * Find preset by name.
 *
 * @param name    Preset name (e.g., "@system-default").
 * @return Pointer to preset structure or NULL if not found.
 */
const struct cli_preset*
cli_preset_find(const char *name);

/**
 * Resolve a preset to its profile identifier and features.
 *
 * Reads the preset configuration file and parses it to extract
 * the profile identifier and features. If additional_features are provided,
 * they are merged with preset features (duplicates removed).
 *
 * @param preset_name          Preset name (e.g., "@system-default").
 * @param additional_features  Additional features to merge with preset features, or NULL.
 * @param _profile_id          Resolved profile identifier (must be freed).
 * @param _features            NULL-terminated array of merged features (must be
 *                             freed with string_array_free), or NULL if not
 *                             requested.
 * @return EOK on success, error code on failure.
 */
errno_t
cli_preset_resolve(const char *preset_name,
                   const char **additional_features,
                   char **_profile_id,
                   char ***_features);

/**
 * Get maximum length of preset names.
 *
 * @return Maximum length of all preset names, 0 if no presets.
 */
int
cli_preset_max_name_length(void);

/**
 * Get list of available presets.
 *
 * Returns an array of only the presets whose configuration files exist.
 * The array is terminated by an entry with NULL name.
 *
 * Caller must free the returned array with @cli_preset_list_free.
 *
 * @return Array of available presets (caller must free),
 *         or NULL on allocation failure.
 */
struct cli_preset*
cli_preset_list(void);

/**
 * Free preset list returned by cli_preset_list().
 *
 * @param list    Preset list to free (can be NULL).
 */
void
cli_preset_list_free(struct cli_preset *list);

#endif /* _PRESETS_H_ */
