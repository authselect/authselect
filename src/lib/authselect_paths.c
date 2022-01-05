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

#include "lib/constants.h"
#include <sys/statvfs.h>

_PUBLIC_ const char *
authselect_path_nsswitch()
{
    return PATH_SYMLINK_NSSWITCH;
}

_PUBLIC_ const char *
authselect_path_systemauth()
{
    return PATH_SYMLINK_SYSTEM;
}

_PUBLIC_ const char *
authselect_path_passwordauth()
{
    return PATH_SYMLINK_PASSWORD;
}

_PUBLIC_ const char *
authselect_path_smartcardauth()
{
    return PATH_SYMLINK_SMARTCARD;
}

_PUBLIC_ const char *
authselect_path_fingerprintauth()
{
    return PATH_SYMLINK_FINGERPRINT;
}

_PUBLIC_ const char *
authselect_path_postlogin()
{
    return PATH_SYMLINK_POSTLOGIN;
}

_PUBLIC_ const char *
authselect_path_dconf_db()
{
    return PATH_SYMLINK_DCONF_DB;
}

_PUBLIC_ const char *
authselect_path_dconf_lock()
{
    return PATH_SYMLINK_DCONF_LOCK;
}

/* Determine if we should maintain a separate "shadow" copy
 * of the config files to detect out of band changes.
 *
 * As of right now, this will return `FALSE` on ostree based systems
 * which have `/var` readonly at *build* time, because that directory
 * is for user state and will not be changed across in-place updates.
 */
errno_t
should_maintain_shadow_copy(bool* should_shadow) {
    struct statvfs stbuf;
    if (statvfs(AUTHSELECT_STATE_DIR, &stbuf) < 0) {
        return errno;
    }
    *should_shadow = (stbuf.f_flag & ST_RDONLY) == 0;
    return EOK;
}
