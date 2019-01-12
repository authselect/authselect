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

    oldmask = umask(AUTHSELECT_FILE_MODE);

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

        ret = file_does_not_link_to(symlinks[i].name, symlinks[i].dest, &valid);
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
