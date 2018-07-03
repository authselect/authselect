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

#ifndef _FILES_H_
#define _FILES_H_

#include <stdbool.h>

#include "common/errno_t.h"

struct authselect_files {
    char *systemauth;
    char *passwordauth;
    char *smartcardauth;
    char *fingerprintauth;
    char *postlogin;
    char *nsswitch;
    char *dconfdb;
    char *dconflock;
};

/**
 * Read information from configuration file.
 *
 * @param _profile_id Profile ID.
 * @param _features   NULL-terminated string array of enabled features.
 *
 * @return EOK on success, ENOENT if the configuration file is missing,
 *         other errno code on error.
 */
errno_t
authselect_config_read(char **_profile_id,
                       char ***_features);


/**
 * Write information into configuration file.
 *
 * @param profile_id Profile ID.
 * @param features   NULL-terminated string array of enabled features.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
authselect_config_write(const char *profile_id,
                        const char **features);

/**
 * Test if all locations that we work with are writable.
 *
 * @return True if all locations are writable, false otherwise.
 */
bool
authselect_config_locations_writable(void);

/**
 * Validate existing configuration.
 *
 * Check that all files are created, readable and with correct content
 * and that all symbolic links exist.
 *
 * @return True if the configuration is valid, false otherwise.
 */
bool
authselect_config_validate_existing(const char *profile_id,
                                    const char **features);

/**
 * Validate non-existing configuration.
 *
 * Check that there are no left overs from previous authselect configuration.
 * All generated files must be removed and all symbolic links must either not
 * exists, point to different location or must be other file or directory.
 *
 * @return True if the are no left overs, false otherwise.
 */
bool
authselect_config_validate_non_existing();

/**
 * Read system files templates and return them in files structure.
 *
 * @param dirname    Name of the directory that contains the files.
 * @param dirfd      File descriptor of the opened directory.
 * @param _templates Output templates.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
authselect_system_read_templates(const char *dirname,
                                 int dirfd,
                                 struct authselect_files **_templates);

/**
 * Generate content of system files based on provided templates.
 *
 * @param features    Optional features that should be enabled.
 * @param templates   System file templates.
 * @param _files      Generated system files content.
 *
 * @return EOK on success, other errno code on failure.
 */
errno_t
authselect_system_generate(const char **features,
                           struct authselect_files *templates,
                           struct authselect_files **_files);

/**
 * Write system files.
 *
 * @param features    Optional features that should be enabled.
 * @param templates   System file templates.
 * @param _files      Generated system files content.
 *
 * @return EOK on success, other errno code on failure.
 */
errno_t
authselect_system_write(const char **features,
                        struct authselect_files *templates);

/**
 * Validate content of system files that we generate.
 *
 * @return True if the files are readable and have expected content, return
 *         false otherwise.
 */
bool
authselect_system_validate(struct authselect_files *files);

/**
 * Validate generated files for non-existing configuration.
 *
 * It checks that there are not left overs from previous authselect
 * configuration, i.e. that all generated files do not exist.
 *
 * @return True if all generated files do not exist, false otherwise.
 */
bool
authselect_system_validate_missing(void);

/**
 * Backup all system configuration files.
 *
 * @param name  Backup name. If not specified, current time with random
 *              suffix will be used.
 * @param _path Path to the backup directory.
 *
 * @return EOK on success, other errno code on failure.
 */
errno_t
authselect_system_backup(const char *backup_name, char **_path);

/**
 * Write symbolic links to system configuration files.
 *
 * @return EOK on success, other errno code on failure.
 */
errno_t
authselect_symlinks_write(void);

/**
 * Validate symbolic links that we create.
 *
 * @return True if all symbolic links points to their expected destination,
 *         false otherwise.
 */
bool
authselect_symlinks_validate(void);

/**
 * Validate symbolic links for non-existing configuration.
 *
 * It checks that all symbolic links are either regular files or they point
 * to different destination that we create for authselect or they do not
 * exist at all.
 *
 * @return True if there are not left over symbolic links, false otherwise.
 */
bool
authselect_symlinks_validate_missing(void);

/**
 * Check if all locations where our symbolic links will be stored
 * are available. Returns false if any file already exist on these
 * locations.
 *
 * @return True if all locations are empty, false otherwise.
 */
bool
authselect_symlinks_location_available(void);

/**
 * List all profile directories in a sorted NULL-terminated string array.
 *
 * This will find all profiles within default, vendor and custom profile
 * directories.
 *
 * @param _profiles NULL-terminated sorted string array.
 *
 * @return EOK on success, other errno code on failure.
 */
errno_t
authselect_profile_list(char ***_profiles);

#endif /* _FILES_H_ */
