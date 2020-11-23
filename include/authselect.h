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

#ifndef _AUTHSELECT_H_
#define _AUTHSELECT_H_

#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Holds information about profile. See authselect_profile_* functions to
 * manipulate this structure.
 */
struct authselect_profile;

/**
 * Holds information about managed files. See authselect_files_* functions to
 * manipulate this structure.
 */
struct authselect_files;

/**
 * Authselect profile types.
 */
enum authselect_profile_type {
    AUTHSELECT_PROFILE_DEFAULT,
    AUTHSELECT_PROFILE_VENDOR,
    AUTHSELECT_PROFILE_CUSTOM,
    AUTHSELECT_PROFILE_ANY
};

/**
 * Activate a profile.
 *
 * Activate profile file @profile_id. If a change to the system configuration
 * not performed by authselect was detected, this operation will fail unless
 * @force_override option is provided.
 *
 * If @force_override option is set to true, authselect will override the
 * changes with its own content.
 *
 * If the selected profile supports optional features, these features
 * may be enabled by putting them into @features array.
 *
 * @param profile_id     Profile identifier.
 * @param features       NULL-terminated array of optional features to enable.
 * @param force_override If true, authselect will override local changes.
 *
 * @return
 * - 0 if the profile is successfully activated.
 * - EEXIST if the system is already configured by other means than
 *   authselect and @force_overwrite must be set to true to force
 *   overwrite the existing files.
 * - ENOENT if the profile was not found.
 * - EINVAL if unsupported feature was provided.
 * - Other errno code on generic error.
 */
int
authselect_activate(const char *profile_id,
                    const char **features,
                    bool force_overwrite);

/**
 * Uninstall authselect configuration.
 *
 * This will copy the files from /etc/authselect to their standard configuration
 * and break the symlinks.
 *
 * @return
 * - 0 if the operation was successful
 * - Other errno code on generic error.
 */
int
authselect_uninstall(void);

/**
 * Apply any changes to currently selected profile.
 *
 * Read currently selected profile together with its enabled features
 * and regenerate existing configuration. This can be used to apply
 * any changes to the profile templates.
 *
 * @return
 * - 0 if the profile is successfully updated.
 * - EEXIST if the system is already configured by other means than
 *   authselect.
 * - ENOENT if there is no existing authselect configuration.
 * - Other errno code on generic error.
 */
int
authselect_apply_changes(void);

/**
 * Backup all system configuration files.
 *
 * @param name          Backup name. If not specified, current time with random
 *                      suffix will be used.
 * @param _path         Path to the backup directory.
 *
 * @return EOK on success, other errno code on failure.
 */
int
authselect_backup(const char *name, char **_path);

/**
 * List available backups.
 *
 * @param _names NULL-terminated string array that holds the backup names.
 *
 * @return EOK on success, other errno code on failure.
 */
char **
authselect_backup_list(void);

/**
 * Remove backup.
 *
 * This will delete the backup configuration files from disk.
 *
 * @param name Backup name.
 *
 * @return EOK on success, other errno code on failure.
 */
int
authselect_backup_remove(const char *name);

/**
 * Restore configuration from backup.
 *
 * This will overwrite current configuration with the old one that is stored
 * in the backup directory.
 *
 * @param name Backup name.
 *
 * @return EOK on success, other errno code on failure.
 */
int
authselect_backup_restore(const char *name);

/**
 * Enable a feature with currently activated profile.
 *
 * @param feature       Feature name to activate.
 *
 * @return
 * - 0 if the feature is successfully enabled.
 * - ENOENT if there is no existing authselect configuration.
 * - Other errno code on generic error.
 */
int
authselect_feature_enable(const char *feature);

/**
 * Disable a feature with currently activated profile.
 *
 * @param feature       Feature name to activate.
 *
 * @return
 * - 0 if the feature is successfully disabled.
 * - ENOENT if there is no existing authselect configuration.
 * - Other errno code on generic error.
 */
int
authselect_feature_disable(const char *feature);

/**
 * Check if current configuration is valid.
 *
 * This will detect any manual changes to current authselect
 * configuration and return its result in @_is_valid.
 *
 * @param _is_valid True if no manual changes were detected, false otherwise.
 *
 * @return
 * - 0 if there is an existing authselect configuration, the result of
 *   validation is returned in @_is_valid output variable.
 * - ENOENT if there is no existing authselect configuration, the result of
 *   validation is returned in @_is_valid output variable.
 * - Other errno code on generic error.
 */
int
authselect_validate_configuration(bool *_is_valid);

/**
 * Return profile identifier and parameters of currently selected profile.
 *
 * Free the returned @_features array with authselect_array_free()
 *
 * @param[out] _profile_id     Profile identifier.
 * @param[out] _features       NULL-terminated array of enabled
 *                             optional features. If NULL, this output parameter
 *                             is ignored.
 *
 * @return
 * - 0 if an configuration was read, current configuration is returned
 *   in @_profile_id and @_features output variables.
 * - ENOENT if there is no existing configuration.
 * - Other errno code on generic error.
 */
int
authselect_current_configuration(char **_profile_id,
                                 char ***_features);

/**
 * Return NULL-terminated array of all available profile identifiers.
 *
 * The array is sorted alphabetically having custom profiles pushed after
 * default and vendor specific profiles.
 *
 * Free the returned array with authselect_array_free()
 *
 * @return NULL-terminated array containing all available profiles or NULL
 * on error.
 */
char **
authselect_list();

/**
 * Return information about a profile.
 *
 * Free the structure with @authselect_profile_free().
 *
 * @param profile_id    Profile identifier.
 * @param _profile      Authselect profile information.
 *
 * @return
 * - 0 if the profile was found and read, profile information
 *   is returned in @_profile output variable.
 * - ENOENT if the profile was not found.
 * - Other errno code on generic error.
 */
int
authselect_profile(const char *profile_id,
                   struct authselect_profile **_profile);

/**
 * Get profile identifier.
 *
 * @param profile    Pointer to structure obtained by @authselect_profile.
 *
 * @return Profile identifier.
 */
const char *
authselect_profile_id(const struct authselect_profile *profile);

/**
 * Get profile name.
 *
 * @param profile    Pointer to structure obtained by @authselect_profile.
 *
 * @return Profile name.
 */
const char *
authselect_profile_name(const struct authselect_profile *profile);

/**
 * Full path to the profile file.
 *
 * @param profile    Pointer to structure obtained by @authselect_profile.
 *
 * @return Profile file path.
 */
const char *
authselect_profile_path(const struct authselect_profile *profile);

/**
 * Get profile description.
 *
 * @param profile    Pointer to structure obtained by @authselect_profile.
 *
 * @return Profile description or NULL if none is available.
 */
const char *
authselect_profile_description(const struct authselect_profile *profile);

/**
 * Get profile requirements for selected features.
 *
 * It is necessary to free the returned pointer manually.
 *
 * @param profile    Pointer to structure obtained by @authselect_profile.
 * @param features   NULL-terminated array of optional features to enable.
 *
 * @return Profile requirements, empty string if there are none requirements
 * or NULL in case of an error.
 */
char *
authselect_profile_requirements(const struct authselect_profile *profile,
                                const char **features);

/**
 * Return nsswitch maps set by the profile.
 *
 * It is necessary to free the returned pointer with @authselect_array_free.
 *
 * @param profile    Pointer to structure obtained by @authselect_profile.
 * @param features   NULL-terminated array of optional features to enable.
 *
 * @return Map names, NULL on error.
 */
char **
authselect_profile_nsswitch_maps(const struct authselect_profile *profile,
                                 const char **features);

/**
 * List features supported by the profile.
 *
 * It is necessary to free the returned pointer with @authselect_array_free.
 *
 * @param profile    Pointer to structure obtained by @authselect_profile.
 *
 * @return NULL-terminated array of supported features, NULL on error.
 */
char **
authselect_profile_features(const struct authselect_profile *profile);

/**
 * Free authconfig_profile structure obtained by @authselect_profile.
 *
 * @param profile    Pointer to structure obtained by @authselect_profile.
 */
void
authselect_profile_free(struct authselect_profile *profile);

/**
 * Get nsswitch and PAM configuration that would be used if this profile
 * would be enabled by @authselect_activate() without performing any
 * changes to the system configuration.
 *
 * Free the files structure with @authselect_files_free().
 *
 * @param profile_id    Profile identifier.
 * @param features      NULL-terminated array of optional features to enable.
 * @param _files        Generated files content.
 *
 * @return
 * - 0 on success, the generated content is returned in @_files output variable.
 * - ENOENT if the profile was not found.
 * - Other errno code on generic error.
 */
int
authselect_files(const char *profile_id,
                 const char **features,
                 struct authselect_files **_files);

/**
 * Get nsswitch.conf content.
 *
 * @param files    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated nsswitch.conf content or NULL if the profile does
 * not touch nsswitch.conf.
 */
const char *
authselect_files_nsswitch(const struct authselect_files *files);

/**
 * Get system-auth pam stack content.
 *
 * @param files    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated system-auth pam stack content or NULL if the profile does
 * not touch this stack.
 */
const char *
authselect_files_systemauth(const struct authselect_files *files);

/**
 * Get password-auth pam stack content.
 *
 * @param files    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated password-auth pam stack content or NULL if the profile does
 * not touch this stack.
 */
const char *
authselect_files_passwordauth(const struct authselect_files *files);

/**
 * Get smartcard-auth pam stack content.
 *
 * @param files    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated smartcard-auth pam stack content or NULL if the profile
 * does not touch this stack.
 */
const char *
authselect_files_smartcardauth(const struct authselect_files *files);

/**
 * Get fingerprint-auth pam stack content.
 *
 * @param files    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated fingerprint-auth pam stack content or NULL if the profile
 * does not touch this stack.
 */
const char *
authselect_files_fingerprintauth(const struct authselect_files *files);

/**
 * Get postlogin pam stack content.
 *
 * @param files    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated postlogin pam stack content or NULL if the profile
 * does not touch this stack.
 */
const char *
authselect_files_postlogin(const struct authselect_files *files);

/**
 * Get dconf database content.
 *
 * @param files    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated dconf database content.
 */
const char *
authselect_files_dconf_db(const struct authselect_files *files);

/**
 * Get dconf lock content.
 *
 * @param files    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated dconf lockcontent.
 */
const char *
authselect_files_dconf_lock(const struct authselect_files *files);

/**
 * Free authconfig_files structure obtained by @authselect_cat.
 *
 * @param files    Pointer to structure obtained by @authselect_cat.
 */
void
authselect_files_free(struct authselect_files *files);

/**
 * @return Path to system nsswitch.conf file.
 */
const char *
authselect_path_nsswitch();

/**
 * @return Path to system system-auth pam stack.
 */
const char *
authselect_path_systemauth();

/**
 * @return Path to system password-auth pam stack.
 */
const char *
authselect_path_passwordauth();

/**
 * @return Path to system smartcard-auth pam stack.
 */
const char *
authselect_path_smartcardauth();

/**
 * @return Path to system fingerprint-auth pam stack.
 */
const char *
authselect_path_fingerprintauth();

/**
 * @return Path to system postlogin pam stack.
 */
const char *
authselect_path_postlogin();

/**
 * @return Path to system dconf database directory.
 */
const char *
authselect_path_dconf_db();

/**
 * @return Path to system dconf locks directory.
 */
const char *
authselect_path_dconf_lock();

/**
 * Empty value for initialization.
 */
#define AUTHSELECT_SYMLINK_NONE     0x0000

/**
 * Create symbolic links for meta files.
 */
#define AUTHSELECT_SYMLINK_META     0x0001

/**
 * Create symbolic links for nsswitch.conf.
 */
#define AUTHSELECT_SYMLINK_NSSWITCH 0x0002

/**
 * Create symbolic links for PAM files.
 */
#define AUTHSELECT_SYMLINK_PAM      0x0004

/**
 * Create symbolic links for dconf files.
 */
#define AUTHSELECT_SYMLINK_DCONF    0x0008

/**
 * Create new profile.
 *
 * Create new profile named @name. Argument @type determines the type of
 * newly created profile and can be either AUTHSELECT_PROFILE_VENDOR or
 * AUTHSELECT_PROFILE_CUSTOM. Default profiles cannot be created as they
 * are only shipped with installation.
 *
 * New profile can be based on an existing profile @base_id. Argument
 * @base_type determines where the base profile is located. If it is
 * AUTHSELECT_PROFILE_ANY, standard algorithm is used, that means if
 * custom/ prefix is is @base_id it is a custom profile, otherwise
 * it is search in vendor and then in default profiles.
 *
 * When the profile is based on @base_id, files from @base_id are copied
 * in the new profile location. Symbolic links can be created instead of
 * creating a copy of a file by providing a bit mask consisting of:
 * - AUTHSELECT_SYMLINK_META
 * - AUTHSELECT_SYMLINK_NSSWITCH
 * - AUTHSELECT_SYMLINK_PAM
 * - AUTHSELECT_SYMLINK_DCONF
 *
 * Alternatively, specific files that should be symlinked can be specified
 * with @symlinks array.
 *
 * @param name           New profile name.
 * @param type           New profile type.
 * @param base_id        Base profile ID.
 * @param base_type      Base profile type.
 * @param symlink_flags  Symbolic links bit mask.
 * @param symlinks       Array of files that should be symlinked.
 * @param _path          Path to the new profile directory.
 *
 * @return
 * - 0 if the profile is successfully created.
 * - EEXIST if the profile already exists.
 * - ENOENT if the base profile is not found.
 * - Other errno code on generic error.
 */
int
authselect_profile_create(const char *name,
                          enum authselect_profile_type type,
                          const char *base_id,
                          enum authselect_profile_type base_type,
                          uint32_t symlink_flags,
                          const char **symlinks,
                          char **_path);

/**
 * Free NULL-terminated string array.
 */
void
authselect_array_free(char **array);

enum authselect_debug {
    AUTHSELECT_INFO,
    AUTHSELECT_WARNING,
    AUTHSELECT_ERROR
};

/* Function to log authselect events.
 *
 * @param pvt      Private data passed to the function.
 * @param level    Debug level.
 * @param function Internal library function name.
 * @param msg      Debug message
 *
 * @see authselect_set_debug_fn
 */
typedef void (*authselect_debug_fn)(void *pvt,
                                    enum authselect_debug level,
                                    const char *file,
                                    unsigned long line,
                                    const char *function,
                                    const char *msg);

/* Set authselect debug function.
 *
 * Only one function can be set at a time. Use NULL to disable debug messages.
 *
 * @param fn Function to be called on internal events.
 * @param pvt Caller private data passed to the function.
 */
void authselect_set_debug_fn(authselect_debug_fn fn, void *pvt);

#endif /* _AUTHSELECT_H_ */
