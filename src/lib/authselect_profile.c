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

#include <string.h>
#include <stdlib.h>

#include "authselect.h"
#include "lib/constants.h"
#include "lib/profiles/profiles.h"

_PUBLIC_ int
authselect_profile(const char *profile_id,
                   struct authselect_profile **_profile)
{
    return authselect_profile_read(profile_id, _profile);
}

_PUBLIC_ const char *
authselect_profile_id(const struct authselect_profile *profile)
{
    if (profile == NULL) {
        return NULL;
    }

    return profile->id;
}

_PUBLIC_ const char *
authselect_profile_name(const struct authselect_profile *profile)
{
    if (profile == NULL) {
        return NULL;
    }

    return profile->name;
}

_PUBLIC_ const char *
authselect_profile_path(const struct authselect_profile *profile)
{
    if (profile == NULL) {
        return NULL;
    }

    return profile->path;
}

_PUBLIC_ const char *
authselect_profile_description(const struct authselect_profile *profile)
{
    if (profile == NULL) {
        return NULL;
    }

    return profile->description;
}

_PUBLIC_ void
authselect_profile_free(struct authselect_profile *profile)
{
    if (profile == NULL) {
        return;
    }

    if (profile->id != NULL) {
        free(profile->id);
    }

    if (profile->path != NULL) {
        free(profile->path);
    }

    if (profile->description != NULL) {
        free(profile->description);
    }

    authselect_files_free(profile->files);

    memset(profile, 0, sizeof(struct authselect_profile));

    free(profile);

    return;
}
