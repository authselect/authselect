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

#ifndef _AUTHSELECT_PATHS_H_
#define _AUTHSELECT_PATHS_H_

/* Authselect configuration file. */
#define PATH_CONFIG_FILE AUTHSELECT_CONFIG_DIR "/authselect.conf"

/* Uid and gid of files owned by authselect. -1 means do not check. */
#define AUTHSELECT_UID -1
#define AUTHSELECT_GID -1

/* Profile file names. */
#define FILE_README      "README"
#define FILE_SYSTEM      "system-auth"
#define FILE_PASSWORD    "password-auth"
#define FILE_FINGERPRINT "fingerprint-auth"
#define FILE_SMARTCARD   "smartcard-auth"
#define FILE_NSSWITCH    "nsswitch.conf"

/* Paths to generated system files. */
#define PATH_SYSTEM      AUTHSELECT_CONFIG_DIR "/" FILE_SYSTEM
#define PATH_PASSWORD    AUTHSELECT_CONFIG_DIR "/" FILE_PASSWORD
#define PATH_FINGERPRINT AUTHSELECT_CONFIG_DIR "/" FILE_FINGERPRINT
#define PATH_SMARTCARD   AUTHSELECT_CONFIG_DIR "/" FILE_SMARTCARD
#define PATH_NSSWITCH    AUTHSELECT_CONFIG_DIR "/" FILE_NSSWITCH
#define PATH_DCONF       AUTHSELECT_DCONF_DIR  "/" AUTHSELECT_DCONF_FILE
#define PATH_DCONF_LOCK  AUTHSELECT_DCONF_DIR  "/locks/" AUTHSELECT_DCONF_FILE

/* Names of symbolic links that points to generated files. */
#define PATH_SYMLINK_SYSTEM      AUTHSELECT_PAM_DIR "/" FILE_SYSTEM
#define PATH_SYMLINK_PASSWORD    AUTHSELECT_PAM_DIR "/" FILE_PASSWORD
#define PATH_SYMLINK_FINGERPRINT AUTHSELECT_PAM_DIR "/" FILE_FINGERPRINT
#define PATH_SYMLINK_SMARTCARD   AUTHSELECT_PAM_DIR "/" FILE_SMARTCARD
#define PATH_SYMLINK_NSSWITCH    AUTHSELECT_NSSWITCH_CONF

/* Path to profile directories. */
#define DIR_DEFAULT_PROFILES AUTHSELECT_PROFILE_DIR "/default"
#define DIR_VENDOR_PROFILES  AUTHSELECT_PROFILE_DIR "/vendor"
#define DIR_CUSTOM_PROFILES  AUTHSELECT_PROFILE_DIR "/custom"

/* Structure to hold path and content of generated system files.
 * @see GENERATED_FILES, GENERATED_FILES_PATHS */
struct authselect_generated {
    const char *path;
    const char *content;
};

#define GENERATED_FILES(files)                                          \
{                                                                       \
    {PATH_SYSTEM,      (files)->systemauth},                            \
    {PATH_PASSWORD,    (files)->passwordauth},                          \
    {PATH_FINGERPRINT, (files)->fingerprintauth},                       \
    {PATH_SMARTCARD,   (files)->smartcardauth},                         \
    {PATH_NSSWITCH,    (files)->nsswitch},                              \
    {PATH_DCONF,       (files)->dconfdb},                               \
    {PATH_DCONF_LOCK,  (files)->dconflock},                             \
    {NULL, NULL}                                                        \
}

#define GENERATED_FILES_PATHS                                           \
{                                                                       \
    {PATH_SYSTEM,      NULL},                                           \
    {PATH_PASSWORD,    NULL},                                           \
    {PATH_FINGERPRINT, NULL},                                           \
    {PATH_SMARTCARD,   NULL},                                           \
    {PATH_NSSWITCH,    NULL},                                           \
    {PATH_DCONF,       NULL},                                           \
    {PATH_DCONF_LOCK,  NULL},                                           \
    {NULL, NULL}                                                        \
}

/* Structure to hold information about symbolic link names and destinations.
 * @see GENERATED_FILES, GENERATED_FILES_PATHS */
struct authselect_symlink {
    const char *name;
    const char *dest;
};

#define SYMLINK_FILES                                                   \
{                                                                       \
    {PATH_SYMLINK_SYSTEM,      PATH_SYSTEM},                            \
    {PATH_SYMLINK_PASSWORD,    PATH_PASSWORD},                          \
    {PATH_SYMLINK_FINGERPRINT, PATH_FINGERPRINT},                       \
    {PATH_SYMLINK_SMARTCARD,   PATH_SMARTCARD},                         \
    {PATH_SYMLINK_NSSWITCH,    PATH_NSSWITCH},                          \
    {NULL, NULL}                                                        \
}

#endif /* _AUTHSELECT_PATHS_H_ */
