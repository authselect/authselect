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
#include <stdbool.h>

#define AUTHSELECT_ERR_FORCE_REQUIRED -1

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
 * Activate a profile.
 *
 * Activate profile file @profile_id. If a change to the system configuration
 * not performed by authselect was detected, this operation will fail unless
 * @force_override option is provided.
 *
 * If @force_override option is set to true, authselect will override the
 * changes with its own content.
 *
 * If the selected profile supports optional modules, these modules
 * may be enabled by putting them into @optional array.
 *
 * @param profile_id     Profile identifier.
 * @param optional       NULL-terminated array of optional modules to enable.
 * @param force_override If true, authselect will override local changes.
 *
 * @return 0 on success, AUTHSELECT_ERR_FORCE_REQUIRED if actication
 * wer errno code on error.
 */
int
authselect_activate(const char *profile_id,
                    const char **optional,
                    bool force_override);

/**
 * Check if current configuration is valid.
 *
 * This will detect any manual changes to current authselect
 * configuration and return its result in @_is_valid.
 *
 * @param _is_valid True if no manual changes were detected, false otherwise.
 *
 * @return EOK on success, errno code on error.
 */
int
authselect_check_conf(bool *_is_valid);

/**
 * Return profile identifier and parameters of currently selected profile.
 *
 * @param[out] _profile_id     Profile identifier.
 * @param[out] _optional       NULL-terminated array of enabled
 *                             optional modules.
 * @param[out] _is_valid       True if the current configuration is valid
 *                             and created by authselect, false if some
 *                             manual changes were detected.
 *
 * @return 0 on success, ENOENT if there is no current configuration present,
 * other errno code on error.
 */
int
authselect_current(char **_profile_id,
                   char ***_optional);

/**
 * Free string array of optional profile paramenters
 * returned by @authselect_current. */
void
authselect_optional_free(char **optional);

/**
 * Return NULL-terminated array of all available profile identifiers.
 *
 * The array is sorted alphabetically having custom profiles pushed after
 * default and vendor specific profiles.
 */
char **
authselect_list();

/**
 * Free string array returned by @authselect_list.
 */
void
authselect_list_free(char **profile_ids);

/**
 * Return information about a profile.
 *
 * @param profile_id    Profile identifier.
 * @param _profile      Authselect profile information.
 *
 * @return 0 on succes, ENOENT if the profile was not found,
 * other errno code on error.
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
 * @param profile_id    Profile identifier.
 * @param optional      NULL-terminated array of optional modules to enable.
 * @param _files        Generated files content.
 *
 * @return 0 on success, ENOENT if the profile was not found,
 * other errno code on error.
 */
int
authselect_cat(const char *profile_id,
               const char **optional,
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
 * Free authconfig_files structure obtained by @authselect_cat.
 *
 * @param files    Pointer to structure obtained by @authselect_cat.
 */
void
authselect_files_free(struct authselect_files *files);

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
