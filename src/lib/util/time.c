/*
    Authors:
        Pavel Březina <pbrezina@redhat.com>

    Copyright (C) 2019 Red Hat

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

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "lib/util/time.h"
#include "common/common.h"

errno_t time_now(time_t *now) {
    char *source_date_epoch;
    unsigned long long epoch;
    char *endptr;

    /* If SOURCE_DATE_EPOCH is set, use it as the current time.
     * This is useful for reproducible builds.
     * See https://reproducible-builds.org/specs/source-date-epoch/ */
    source_date_epoch = getenv("SOURCE_DATE_EPOCH");
    if (source_date_epoch) {
        INFO("Using time from SOURCE_DATE_EPOCH: %s", source_date_epoch);
        errno = 0;
        epoch = strtoull(source_date_epoch, &endptr, 10);
        if ((errno == ERANGE && (epoch == ULLONG_MAX || epoch == 0)) ||
            (errno != 0 && epoch == 0)) {
            ERROR("Environment variable $SOURCE_DATE_EPOCH: strtoull: %s\n",
                  strerror(errno));
            return EINVAL;
        }
        if (endptr == source_date_epoch) {
            ERROR(
                "Environment variable $SOURCE_DATE_EPOCH: No digits were "
                "found: %s\n",
                endptr);
            return EINVAL;
        }
        if (*endptr != '\0') {
            ERROR(
                "Environment variable $SOURCE_DATE_EPOCH: Trailing garbage: "
                "%s\n",
                endptr);
            return EINVAL;
        }
        if (epoch > ULONG_MAX) {
            ERROR(
                "Environment variable $SOURCE_DATE_EPOCH: value must be "
                "smaller than or equal to %lu but was found to be: %llu \n",
                ULONG_MAX, epoch);
            return EINVAL;
        }
        *now = epoch;
    } else {
        INFO("Using system time");
        *now = time(NULL);
    }
    return EOK;
}
