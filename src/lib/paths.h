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

#include <stdbool.h>

/* Authselect configuration file. */
#define FILE_CONFIG      "authselect.conf"
#define PATH_CONFIG_FILE AUTHSELECT_CONFIG_DIR "/" FILE_CONFIG

/* UID and GID of files owned by authselect. -1 means do not check. */
#define AUTHSELECT_UID -1
#define AUTHSELECT_GID -1

/* Profile file names. */
#define FILE_README      "README"
#define FILE_REQUIREMENT "REQUIREMENTS"
#define FILE_SYSTEM      "system-auth"
#define FILE_PASSWORD    "password-auth"
#define FILE_FINGERPRINT "fingerprint-auth"
#define FILE_SMARTCARD   "smartcard-auth"
#define FILE_SWITCHABLE  "switchable-auth"
#define FILE_POSTLOGIN   "postlogin"
#define FILE_NSSWITCH    "nsswitch.conf"
#define FILE_DCONF_DB    "dconf-db"
#define FILE_DCONF_LOCK  "dconf-locks"

/* Paths to generated system files. */
#define PATH_SYSTEM      AUTHSELECT_CONFIG_DIR "/" FILE_SYSTEM
#define PATH_PASSWORD    AUTHSELECT_CONFIG_DIR "/" FILE_PASSWORD
#define PATH_FINGERPRINT AUTHSELECT_CONFIG_DIR "/" FILE_FINGERPRINT
#define PATH_SMARTCARD   AUTHSELECT_CONFIG_DIR "/" FILE_SMARTCARD
#define PATH_SWITCHABLE  AUTHSELECT_CONFIG_DIR "/" FILE_SWITCHABLE
#define PATH_POSTLOGIN   AUTHSELECT_CONFIG_DIR "/" FILE_POSTLOGIN
#define PATH_NSSWITCH    AUTHSELECT_CONFIG_DIR "/" FILE_NSSWITCH
#define PATH_DCONF_DB    AUTHSELECT_CONFIG_DIR "/" FILE_DCONF_DB
#define PATH_DCONF_LOCK  AUTHSELECT_CONFIG_DIR "/" FILE_DCONF_LOCK

/* Names of symbolic links that points to generated files. */
#define PATH_SYMLINK_SYSTEM      AUTHSELECT_PAM_DIR "/" FILE_SYSTEM
#define PATH_SYMLINK_PASSWORD    AUTHSELECT_PAM_DIR "/" FILE_PASSWORD
#define PATH_SYMLINK_FINGERPRINT AUTHSELECT_PAM_DIR "/" FILE_FINGERPRINT
#define PATH_SYMLINK_SMARTCARD   AUTHSELECT_PAM_DIR "/" FILE_SMARTCARD
#define PATH_SYMLINK_SWITCHABLE  AUTHSELECT_PAM_DIR "/" FILE_SWITCHABLE
#define PATH_SYMLINK_POSTLOGIN   AUTHSELECT_PAM_DIR "/" FILE_POSTLOGIN
#define PATH_SYMLINK_NSSWITCH    AUTHSELECT_NSSWITCH_CONF
#define PATH_SYMLINK_DCONF_DB    AUTHSELECT_DCONF_DIR  "/" AUTHSELECT_DCONF_FILE
#define PATH_SYMLINK_DCONF_LOCK  AUTHSELECT_DCONF_DIR  "/locks/" AUTHSELECT_DCONF_FILE

/* Path to profile directories. */
#define DIR_DEFAULT_PROFILES AUTHSELECT_PROFILE_DIR
#define DIR_VENDOR_PROFILES  AUTHSELECT_VENDOR_DIR
#define DIR_CUSTOM_PROFILES  AUTHSELECT_CUSTOM_DIR

/* Structure to hold path and content of generated system files.
 * @see GENERATED_FILES, GENERATED_FILES_PATHS */
struct authselect_generated {
    const char *path;
    const char *content;
};

#define GENERATED_FILES(files)                                       \
{                                                                    \
    {PATH_SYSTEM,      (files)->systemauth},                         \
    {PATH_PASSWORD,    (files)->passwordauth},                       \
    {PATH_FINGERPRINT, (files)->fingerprintauth},                    \
    {PATH_SMARTCARD,   (files)->smartcardauth},                      \
    {PATH_SWITCHABLE,  (files)->switchableauth},                    \
    {PATH_POSTLOGIN,   (files)->postlogin},                          \
    {PATH_NSSWITCH,    (files)->nsswitch},                           \
    {PATH_DCONF_DB,    (files)->dconfdb},                            \
    {PATH_DCONF_LOCK,  (files)->dconflock},                          \
    {NULL, NULL}                                                     \
}

#define GENERATED_FILES_PATHS                                        \
{                                                                    \
    {PATH_SYSTEM,      NULL},                                        \
    {PATH_PASSWORD,    NULL},                                        \
    {PATH_FINGERPRINT, NULL},                                        \
    {PATH_SMARTCARD,   NULL},                                        \
    {PATH_SWITCHABLE,  NULL},                                        \
    {PATH_POSTLOGIN,   NULL},                                        \
    {PATH_NSSWITCH,    NULL},                                        \
    {PATH_DCONF_DB,    NULL},                                        \
    {PATH_DCONF_LOCK,  NULL},                                        \
    {NULL, NULL}                                                     \
}

#define PROFILE_FILES(files)                                         \
{                                                                    \
    {FILE_SYSTEM,      (files)->systemauth},                         \
    {FILE_PASSWORD,    (files)->passwordauth},                       \
    {FILE_FINGERPRINT, (files)->fingerprintauth},                    \
    {FILE_SMARTCARD,   (files)->smartcardauth},                      \
    {FILE_SWITCHABLE,  (files)->switchableauth},                     \
    {FILE_POSTLOGIN,   (files)->postlogin},                          \
    {FILE_NSSWITCH,    (files)->nsswitch},                           \
    {FILE_DCONF_DB,    (files)->dconfdb},                            \
    {FILE_DCONF_LOCK,  (files)->dconflock},                          \
    {NULL, NULL}                                                     \
}

/* Structure to hold information about symbolic link names and destinations.
 * @see GENERATED_FILES, GENERATED_FILES_PATHS */
struct authselect_symlink {
    /* Name of the symbolic link. */
    const char *name;
    /* Its destination. */
    const char *dest;

    /* If false, only a warning is yield if its parent directory does not
     * exist. Otherwise it is considered as error. */
    bool create_path;
};

#define SYMLINK_FILES                                                   \
    {PATH_SYMLINK_SYSTEM,      PATH_SYSTEM,      false},                \
    {PATH_SYMLINK_PASSWORD,    PATH_PASSWORD,    false},                \
    {PATH_SYMLINK_FINGERPRINT, PATH_FINGERPRINT, false},                \
    {PATH_SYMLINK_SMARTCARD,   PATH_SMARTCARD,   false},                \
    {PATH_SYMLINK_SWITCHABLE,  PATH_SWITCHABLE,  false},                \
    {PATH_SYMLINK_POSTLOGIN,   PATH_POSTLOGIN,   false},                \
    {PATH_SYMLINK_NSSWITCH,    PATH_NSSWITCH,    false},                \
    {PATH_SYMLINK_DCONF_DB,    PATH_DCONF_DB,    true},                 \
    {PATH_SYMLINK_DCONF_LOCK,  PATH_DCONF_LOCK,  true},                 \
    {NULL, NULL, false}                                                 \

/**
 * Profile files grouped by their purpose.
 */
#define FILES_META      FILE_README, FILE_REQUIREMENT
#define FILES_NSSWITCH  FILE_NSSWITCH
#define FILES_PAM       FILE_SYSTEM, FILE_PASSWORD, FILE_FINGERPRINT,   \
                        FILE_SMARTCARD, FILE_SWITCHABLE, FILE_POSTLOGIN
#define FILES_DCONF     FILE_DCONF_DB, FILE_DCONF_LOCK

#define FILES_ALL       FILES_META, FILES_NSSWITCH, FILES_PAM, FILES_DCONF

#endif /* _AUTHSELECT_PATHS_H_ */
