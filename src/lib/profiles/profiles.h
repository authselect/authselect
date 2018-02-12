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

#ifndef _PROFILES_H_
#define _PROFILES_H_

#include "common/common.h"

/**
 * Activate given profile.
 *
 * Write all changes to the system.
 *
 * @return EOK on success, EACCES if we can not access some directories,
 *         other errno code on error.
 */
errno_t
authselect_profile_activate(struct authselect_profile *profile,
                            const char **features);

/**
 * List all profile directories in a sorted NULL-terminated string array.
 *
 * This will find all profiles within default, vendor and custom profile
 * directories.
 *
 * @param _profiles     NULL-terminated sorted string array.
 *
 * @return EOK on success, other errno code on failure.
 */
errno_t
authselect_profile_list(char ***_profiles);

/**
 * Read profile information.
 *
 * @param profile_id    Profile ID to search for.
 * @param _profile      Profile information.
 *
 * @return EOK on success, ENOENT if the profile was not found, other errno
 *         code on error.
 */
errno_t
authselect_profile_read(const char *profile_id,
                        struct authselect_profile **_profile);

/**
 * Create custom profile id from a directory name.
 *
 * @param name          Profile directory name.
 *
 * @return Custom profile id or NULL if allocation fails.
 */
char *
authselect_profile_custom_id(const char *name);

/**
 * Return directory name of a custom profile.
 *
 * @param profile_id    Profile id.
 *
 * @return Directory name of the profile, NULL if it is not a custom profile.
 */
const char *
authselect_profile_parse_custom(const char *profile_id);

/**
 * @return True if @profile_id is a custom profile, false otherwise.
 */
bool
authselect_profile_is_custom(const char *profile_id);

#endif /* _PROFILES_H_ */
