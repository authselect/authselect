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

struct authselect_profile;
struct authselect_cat;

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
 * @return 0 on success, errno code on error.
 */
int
authselect_activate(const char *profile_id,
                    const char **optional,
                    bool force_override);

/**
 * Return profile identifier of currently selected profile
 * or NULL if the system is not configured by authselect.
 */
const char *
authselect_current();

/**
 * Return NULL-terminated array of all available profile identifiers.
 */
const char **
authselect_list();

/**
 * Return information about a profile.
 *
 * @param profile_id    Profile identifier.
 *
 * @return Authselect profile information or NULL on error.
 */
struct authselect_profile *
authselect_profile(const char *profile_id);

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
 * Get profile short description.
 *
 * @param profile    Pointer to structure obtained by @authselect_profile.
 *
 * @return Profile short description or NULL if none is available.
 */
const char *
authselect_profile_short_description(const struct authselect_profile *profile);

/**
 * Get profile full description.
 *
 * @param profile    Pointer to structure obtained by @authselect_profile.
 *
 * @return Profile full description or NULL if none is available.
 */
const char *
authselect_profile_full_description(const struct authselect_profile *profile);

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
 *
 * @return Resulting content or NULL on error.
 */
struct authselect_cat *
authselect_cat(const char *profile,
               const char **optional);

/**
 * Get nsswitch.conf content.
 *
 * @param cat    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated nsswitch.conf content or NULL if the profile does
 * not touch nsswitch.conf.
 */
const char *
authselect_cat_nsswitch(const struct authselect_cat *cat);

/**
 * Get system-auth pam stack content.
 *
 * @param cat    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated system-auth pam stack content or NULL if the profile does
 * not touch this stack.
 */
const char *
authselect_cat_systemauth(const struct authselect_cat *cat);

/**
 * Get password-auth pam stack content.
 *
 * @param cat    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated password-auth pam stack content or NULL if the profile does
 * not touch this stack.
 */
const char *
authselect_cat_passwordauth(const struct authselect_cat *cat);

/**
 * Get smartcard-auth pam stack content.
 *
 * @param cat    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated smartcard-auth pam stack content or NULL if the profile
 * does not touch this stack.
 */
const char *
authselect_cat_smartcardauth(const struct authselect_cat *cat);

/**
 * Get fingerprint-auth pam stack content.
 *
 * @param cat    Pointer to structure obtained by @authselect_cat.
 *
 * @return Generated fingerprint-auth pam stack content or NULL if the profile
 * does not touch this stack.
 */
const char *
authselect_cat_fingerprintauth(const struct authselect_cat *cat);

/**
 * Free authconfig_cat structure obtained by @authselect_cat.
 *
 * @param cat    Pointer to structure obtained by @authselect_cat.
 */
void
authselect_cat_free(struct authselect_cat *cat);

#endif /* _AUTHSELECT_H_ */
