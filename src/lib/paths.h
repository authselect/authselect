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
#define PATH_CONFIG_FILE AUTHSELECT_CONFIG_DIR "/authselect.conf"

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
#define FILE_POSTLOGIN   "postlogin"
#define FILE_NSSWITCH    "nsswitch.conf"
#define FILE_DCONF_DB    "dconf-db"
#define FILE_DCONF_LOCK  "dconf-locks"

/* Paths to generated system files. */
#define PATH_SYSTEM      AUTHSELECT_CONFIG_DIR "/" FILE_SYSTEM
#define PATH_PASSWORD    AUTHSELECT_CONFIG_DIR "/" FILE_PASSWORD
#define PATH_FINGERPRINT AUTHSELECT_CONFIG_DIR "/" FILE_FINGERPRINT
#define PATH_SMARTCARD   AUTHSELECT_CONFIG_DIR "/" FILE_SMARTCARD
#define PATH_POSTLOGIN   AUTHSELECT_CONFIG_DIR "/" FILE_POSTLOGIN
#define PATH_NSSWITCH    AUTHSELECT_CONFIG_DIR "/" FILE_NSSWITCH
#define PATH_DCONF_DB    AUTHSELECT_CONFIG_DIR "/" FILE_DCONF_DB
#define PATH_DCONF_LOCK  AUTHSELECT_CONFIG_DIR "/" FILE_DCONF_LOCK

/* Paths to copy generated system files. Used to check changes
 * in configuration. */
#define PATH_COPY_SYSTEM      AUTHSELECT_STATE_DIR "/" FILE_SYSTEM
#define PATH_COPY_PASSWORD    AUTHSELECT_STATE_DIR "/" FILE_PASSWORD
#define PATH_COPY_FINGERPRINT AUTHSELECT_STATE_DIR "/" FILE_FINGERPRINT
#define PATH_COPY_SMARTCARD   AUTHSELECT_STATE_DIR "/" FILE_SMARTCARD
#define PATH_COPY_POSTLOGIN   AUTHSELECT_STATE_DIR "/" FILE_POSTLOGIN
#define PATH_COPY_NSSWITCH    AUTHSELECT_STATE_DIR "/" FILE_NSSWITCH
#define PATH_COPY_DCONF_DB    AUTHSELECT_STATE_DIR "/" FILE_DCONF_DB
#define PATH_COPY_DCONF_LOCK  AUTHSELECT_STATE_DIR "/" FILE_DCONF_LOCK

/* Names of symbolic links that points to generated files. */
#define PATH_SYMLINK_SYSTEM      AUTHSELECT_PAM_DIR "/" FILE_SYSTEM
#define PATH_SYMLINK_PASSWORD    AUTHSELECT_PAM_DIR "/" FILE_PASSWORD
#define PATH_SYMLINK_FINGERPRINT AUTHSELECT_PAM_DIR "/" FILE_FINGERPRINT
#define PATH_SYMLINK_SMARTCARD   AUTHSELECT_PAM_DIR "/" FILE_SMARTCARD
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
    const char *copy_path;
    const char *content;
};

#define GENERATED_FILES(files)                                             \
{                                                                          \
    {PATH_SYSTEM,      PATH_COPY_SYSTEM,      (files)->systemauth},        \
    {PATH_PASSWORD,    PATH_COPY_PASSWORD,    (files)->passwordauth},      \
    {PATH_FINGERPRINT, PATH_COPY_FINGERPRINT, (files)->fingerprintauth},   \
    {PATH_SMARTCARD,   PATH_COPY_SMARTCARD,   (files)->smartcardauth},     \
    {PATH_POSTLOGIN,   PATH_COPY_POSTLOGIN,   (files)->postlogin},         \
    {PATH_NSSWITCH,    PATH_COPY_NSSWITCH,    (files)->nsswitch},          \
    {PATH_DCONF_DB,    PATH_COPY_DCONF_DB,    (files)->dconfdb},           \
    {PATH_DCONF_LOCK,  PATH_COPY_DCONF_LOCK,  (files)->dconflock},         \
    {NULL, NULL, NULL}                                                     \
}

#define GENERATED_FILES_PATHS                                              \
{                                                                          \
    {PATH_SYSTEM,      NULL, NULL},                                        \
    {PATH_PASSWORD,    NULL, NULL},                                        \
    {PATH_FINGERPRINT, NULL, NULL},                                        \
    {PATH_SMARTCARD,   NULL, NULL},                                        \
    {PATH_POSTLOGIN,   NULL, NULL},                                        \
    {PATH_NSSWITCH,    NULL, NULL},                                        \
    {PATH_DCONF_DB,    NULL, NULL},                                        \
    {PATH_DCONF_LOCK,  NULL, NULL},                                        \
    {NULL, NULL, NULL}                                                     \
}

#define PROFILE_FILES(files)                                            \
{                                                                       \
    {FILE_SYSTEM,      NULL, (files)->systemauth},                      \
    {FILE_PASSWORD,    NULL, (files)->passwordauth},                    \
    {FILE_FINGERPRINT, NULL, (files)->fingerprintauth},                 \
    {FILE_SMARTCARD,   NULL, (files)->smartcardauth},                   \
    {FILE_POSTLOGIN,   NULL, (files)->postlogin},                       \
    {FILE_NSSWITCH,    NULL, (files)->nsswitch},                        \
    {FILE_DCONF_DB,    NULL, (files)->dconfdb},                         \
    {FILE_DCONF_LOCK,  NULL, (files)->dconflock},                       \
    {NULL, NULL, NULL}                                                  \
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
                        FILE_SMARTCARD, FILE_POSTLOGIN
#define FILES_DCONF     FILE_DCONF_DB, FILE_DCONF_LOCK

#define FILES_ALL       FILES_META, FILES_NSSWITCH, FILES_PAM, FILES_DCONF

#endif /* _AUTHSELECT_PATHS_H_ */
