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

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "common/common.h"
#include "lib/constants.h"
#include "lib/util/util.h"
#include "lib/files/files.h"

errno_t
authselect_symlinks_write()
{
    struct authselect_symlink symlinks[] = {SYMLINK_FILES};
    mode_t oldmask;
    errno_t ret;
    int i;

    oldmask = umask(~AUTHSELECT_FILE_MODE & ALLPERMS);

    for (i = 0; symlinks[i].name != NULL; i++) {
        INFO("Creating symbolic link [%s] to [%s]",
             symlinks[i].name, symlinks[i].dest);

        ret = unlink(symlinks[i].name);
        if (ret != 0 && errno != ENOENT) {
            ret = errno;
            ERROR("Unable to overwrite file [%s] [%d]: %s",
                  symlinks[i].name, ret, strerror(ret));
            goto done;
        }

        ret = symlink(symlinks[i].dest, symlinks[i].name);
        if (ret != 0) {
            ret = errno;
            ERROR("Unable to create symbolic link [%s] [%d]: %s",
                  symlinks[i].name, ret, strerror(ret));
            goto done;
        }
    }

    ret = EOK;

done:
    umask(oldmask);

    return ret;
}

bool
authselect_symlinks_validate()
{
    struct authselect_symlink symlinks[] = {SYMLINK_FILES};
    bool result = true;
    bool is_valid;
    errno_t ret;
    int i;

    for (i = 0; symlinks[i].name != NULL; i++) {
        INFO("Validating link [%s]", symlinks[i].name);

        ret = file_links_to(symlinks[i].name, symlinks[i].dest, &is_valid);
        if (ret != EOK) {
            ERROR("Unable to validate link [%s] [%d]: %s",
                  symlinks[i].name, ret, strerror(ret));
            result = false;
            continue;
        }

        if (!is_valid) {
            ERROR("[%s] was not created by authselect!", symlinks[i].name);
            result = false;
        }
    }

    return result;
}

bool
authselect_symlinks_validate_missing()
{
    struct authselect_symlink symlinks[] = {SYMLINK_FILES};
    bool result = true;
    bool valid;
    errno_t ret;
    int i;

    for (i = 0; symlinks[i].name != NULL; i++) {
        ret = file_exists(symlinks[i].name);
        if (ret == ENOENT) {
            continue;
        } else if (ret != EOK) {
            ERROR("Error while trying to access file [%s] [%d]: %s",
                  symlinks[i].name, ret, strerror(ret));
            result = false;
            continue;
        }

        ret = file_does_not_link_to(symlinks[i].name, symlinks[i].dest, true,
                                    &valid);
        if (ret != EOK) {
            ERROR("Unable to check file [%s] [%d]: %s",
                  symlinks[i].name, ret, strerror(ret));
            result = false;
            continue;
        }

        if (!valid) {
            ERROR("Symbolic link [%s] to [%s] still exists!",
                  symlinks[i].name, symlinks[i].dest);
            result = false;
            continue;
        }
    }

    return result;
}

bool
authselect_symlinks_location_available()
{
    struct authselect_symlink symlinks[] = {SYMLINK_FILES};
    bool result = true;
    errno_t ret;
    int i;

    for (i = 0; symlinks[i].name != NULL; i++) {
        ret = file_exists(symlinks[i].name);
        if (ret == EOK) {
            ERROR("File [%s] exists but it needs to be overwritten!",
                  symlinks[i].name);
            result = false;
        } else if (ret != ENOENT) {
            ERROR("Error while trying to access file [%s] [%d]: %s",
                  symlinks[i].name, ret, strerror(ret));
            result = false;
        }
    }

    return result;
}

errno_t
authselect_symlinks_uninstall()
{
    struct selinux_safe_copy table[] = {
        {PATH_SYSTEM,      PATH_SYMLINK_SYSTEM, false, false},
        {PATH_PASSWORD,    PATH_SYMLINK_PASSWORD, false, false},
        {PATH_FINGERPRINT, PATH_SYMLINK_FINGERPRINT, false, false},
        {PATH_SMARTCARD,   PATH_SYMLINK_SMARTCARD, false, false},
        {PATH_SWITCHABLE,  PATH_SYMLINK_SWITCHABLE, false, false},
        {PATH_POSTLOGIN,   PATH_SYMLINK_POSTLOGIN, false, false},
        {PATH_NSSWITCH,    PATH_SYMLINK_NSSWITCH, false, false},
        {PATH_DCONF_DB,    PATH_SYMLINK_DCONF_DB, false, false},
        {PATH_DCONF_LOCK,  PATH_SYMLINK_DCONF_LOCK, false, false},
        {NULL, NULL, false, false}
    };
    errno_t ret;
    bool result;
    int i;

    for (i = 0; table[i].source != NULL; i++) {
        /* Check if the symlink is still valid. */
        ret = file_does_not_link_to(table[i].destination, table[i].source,
                                    false, &result);
        if (ret != EOK) {
            return ret;
        }

        if (result) {
            /* This path is already not part of authselect. Let's skip it. */
            INFO("Skipping [%s] because it is not an authselect file",
                 table[i].destination);
            table[i].destination = NULL;
        }
    }

    return selinux_copy_files_safely(table, AUTHSELECT_DIR_MODE, false);
}
