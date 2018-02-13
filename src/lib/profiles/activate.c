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
#include <string.h>

#include "common/common.h"
#include "lib/constants.h"
#include "lib/files/files.h"
#include "lib/profiles/profiles.h"

errno_t
authselect_profile_activate(struct authselect_profile *profile,
                            const char **features)
{
    errno_t ret;

    /* Check that all directories are writable. */
    if (!authselect_config_locations_writable()) {
        ERROR("Some directories are not accessible by authselect!");
        return EACCES;
    }

    ret = authselect_system_write(features, profile->files);
    if (ret != EOK) {
        ERROR("Unable to write generated system files [%d]: %s",
              ret, strerror(ret));
        goto done;
    }

    ret = authselect_config_write(profile->id, features);
    if (ret != EOK) {
        ERROR("Unable to write configuration [%d]: %s", ret, strerror(ret));
        goto done;
    }

    ret = authselect_symlinks_write();
    if (ret != EOK) {
        ERROR("Unable to create symbolic links [%d]: %s", ret, strerror(ret));
        goto done;
    }

done:
    return ret;
}
