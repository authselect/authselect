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

#include <string.h>

#include "common/common.h"
#include "lib/authselect_private.h"

char *
authselect_profile_custom_id(const char *name)
{
    return format("%s%s", AUTHSELECT_CUSTOM_PREFIX, name);
}

const char *
authselect_profile_parse_custom(const char *profile_id)
{
    static size_t len = strlen(AUTHSELECT_CUSTOM_PREFIX);

    if (profile_id == NULL) {
        return NULL;
    }

    if (strncmp(AUTHSELECT_CUSTOM_PREFIX, profile_id, len) != 0) {
        return NULL;
    }

    return profile_id + len;
}

bool
authselect_profile_is_custom(const char *profile_id)
{
    return authselect_profile_parse_custom(profile_id) != NULL;
}
