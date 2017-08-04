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

#ifndef _AUTHSELECT_PRIVATE_H_
#define _AUTHSELECT_PRIVATE_H_

#include "config.h"

#include <dirent.h>

#include "authselect_util.h"

struct authselect_dir;

struct authselect_files {
    char *systemauth;
    char *passwordauth;
    char *smartcardauth;
    char *fingerprintauth;
    char *nsswitch;
};

struct authselect_profile {
    char *id;
    char *path;

    char *name;
    char *description;

    struct authselect_files files;
};

/**
 * Free authselect_dir structure.
 */
void authselect_dir_free(struct authselect_dir *dir);

/**
 * Read profile identifiers from directory.
 *
 * @param dirpath Path to the profile directory.
 * @param _dir    Output authselect_dir object.
 *
 * @return EOK on success, errno code on failure.
 */
errno_t authselect_dir_read(const char *dirpath, struct authselect_dir **_dir);

/**
 * Merge profile identifiers from three authselect directories.
 *
 * - All profiles from @profile will be included.
 * - Only profile from @vendor that are not present in @profile will be added.
 * - All profiles from @custom will be added having their name prefixed with
 *   CUSTOM_PROFILE_PREFIX
 *
 * @return NULL-terminated array of profile identifiers or NULL on error.
 */
char **
authselect_merge_profiles(struct authselect_dir *profile,
                          struct authselect_dir *vendor,
                          struct authselect_dir *custom);

/**
 * Build custom profile identifier from @name.
 *
 * @return CUSTOM_PROFILE_PREFIX concatenated with @name or NULL on error.
 */
char *authselect_profile_custom_id(const char *profile_dirname);

/**
 * Free authselect_files structure content but not the structure itself.
 */
void
authselect_files_free_content(struct authselect_files *files);

/**
 * Generate output files for given profile.
 *
 * @param profile  An authselect profile.
 * @param optional Profile optional modules to enable.
 * @param _file    Generated files.
 *
 * @return EOK on success, errno code on failure.
 */
errno_t
authselect_files_generate(struct authselect_profile *profile,
                          const char **optional,
                          struct authselect_files **_files);

/**
 * Read information from configuration file.
 *
 * @param confpath Path to the configuration file.
 * @param _profile_id     Profile identifier.
 * @param _optional       NULL-terminated array of enabled
 *                             optional modules.
 * @return EOK on success, errno code on error.
 */
errno_t
authselect_read_conf(char **_profile_id,
                     char ***_optional);

/**
 * Check if authselect symbolic links exist or not and save the result
 * in @_exist.
 *
 * @param _exist True if any symbolic link was foud, false otherwise.
 *
 * @return EOK on success, errno code on error.
 */
errno_t
authselect_check_symlinks_presence(bool *_exist);

#endif /* _AUTHSELECT_PRIVATE_H_ */
