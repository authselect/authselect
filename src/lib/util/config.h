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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "common/common.h"

/**
 * Read configuration file containing profile ID and features.
 *
 * The file format is:
 *   Line 1: profile ID
 *   Lines 2+: features (one per line, empty lines ignored)
 *
 * @param path          Path to configuration file.
 * @param _profile_id   Output: profile identifier (must be freed).
 * @param _features     Output: NULL-terminated array of features (must be freed
 *                      with string_array_free), or NULL if not requested.
 * @return EOK on success, error code on failure.
 */
errno_t
config_read(const char *path,
            char **_profile_id,
            char ***_features);

#endif /* _CONFIG_H_ */
