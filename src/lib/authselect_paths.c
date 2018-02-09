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

#include "lib/authselect_paths.h"
#include "lib/authselect_private.h"

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
