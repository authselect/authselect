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

#ifndef _AUTHSELECT_FILES_H_
#define _AUTHSELECT_FILES_H_

#include "config.h"

/* Authselect configuration file. */
#define PATH_CONFIG_FILE AUTHSELECT_CONFIG_DIR "/authselect.conf"

/* Uid and gid of files owned by authselect. */
#define AUTHSELECT_UID -1
#define AUTHSELECT_GID -1

/* Profile files. */
#define FILE_README      "README"
#define FILE_SYSTEM      "system-auth"
#define FILE_PASSWORD    "password-auth"
#define FILE_FINGERPRINT "fingerprint-auth"
#define FILE_SMARTCARD   "smartcard-auth"
#define FILE_NSSWITCH    "nsswitch.conf"

/* Generated files. */
#define PATH_SYSTEM      AUTHSELECT_CONFIG_DIR "/" FILE_SYSTEM
#define PATH_PASSWORD    AUTHSELECT_CONFIG_DIR "/" FILE_PASSWORD
#define PATH_FINGERPRINT AUTHSELECT_CONFIG_DIR "/" FILE_FINGERPRINT
#define PATH_SMARTCARD   AUTHSELECT_CONFIG_DIR "/" FILE_SMARTCARD
#define PATH_NSSWITCH    AUTHSELECT_CONFIG_DIR "/" FILE_NSSWITCH

/* PAM directory symlinks. */
#define SYMLINK_SYSTEM      "system-auth"
#define SYMLINK_PASSWORD    "password-auth"
#define SYMLINK_FINGERPRINT "fingerprint-auth"
#define SYMLINK_SMARTCARD   "smartcard-auth"
#define SYMLINK_NSSWITCH    "nsswitch.conf"

#define PATH_SYMLINK_SYSTEM      AUTHSELECT_PAM_DIR "/" SYMLINK_SYSTEM
#define PATH_SYMLINK_PASSWORD    AUTHSELECT_PAM_DIR "/" SYMLINK_PASSWORD
#define PATH_SYMLINK_FINGERPRINT AUTHSELECT_PAM_DIR "/" SYMLINK_FINGERPRINT
#define PATH_SYMLINK_SMARTCARD   AUTHSELECT_PAM_DIR "/" SYMLINK_SMARTCARD
#define PATH_SYMLINK_NSSWITCH    AUTHSELECT_PAM_DIR "/" SYMLINK_NSSWITCH

#define DIR_DEFAULT_PROFILES AUTHSELECT_PROFILE_DIR "/default"
#define DIR_VENDOR_PROFILES AUTHSELECT_PROFILE_DIR "/vendor"
#define DIR_CUSTOM_PROFILES AUTHSELECT_PROFILE_DIR "/custom"

#define GENERATED_FILES \
{                       \
    PATH_SYSTEM,        \
    PATH_PASSWORD,      \
    PATH_FINGERPRINT,   \
    PATH_SMARTCARD,     \
    PATH_NSSWITCH,      \
    NULL                \
}

struct authselect_symlink {
    const char *name;
    const char *dest;
};

#define SYMLINK_FILES                                  \
{                                                      \
    {PATH_SYMLINK_SYSTEM,      PATH_SYSTEM},           \
    {PATH_SYMLINK_PASSWORD,    PATH_PASSWORD},         \
    {PATH_SYMLINK_FINGERPRINT, PATH_FINGERPRINT},      \
    {PATH_SYMLINK_SMARTCARD,   PATH_SMARTCARD},        \
    {PATH_SYMLINK_NSSWITCH,    PATH_NSSWITCH},         \
    {NULL, NULL}                                       \
}

#endif /* _AUTHSELECT_FILES_H_ */
