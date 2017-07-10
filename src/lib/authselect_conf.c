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
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "authselect_util.h"
#include "authselect_files.h"
#include "authselect_private.h"

static errno_t
trim(const char *str, char **_trimmed)
{
    char *dup;
    const char *end;

    /* Trim beginning. */
    while (isspace(*str)) {
        str++;
    }

    if (str[0] == '\0') {
        *_trimmed = NULL;
        return EOK;
    }

    /* Trim end. */
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) {
        end--;
    }

    dup = strndup(str, end - str + 1);
    if (dup == NULL) {
        return ENOMEM;
    }

    *_trimmed = dup;

    return EOK;
}

static errno_t
read_line(FILE *file, char **_line)
{
    char *line = NULL;
    size_t len = 0;
    char *trimmed;
    errno_t ret;

    errno = 0;
    while (getline(&line, &len, file) != -1) {
        ret = trim(line, &trimmed);

        /* Reset valus for next getline call. */
        free(line);
        line = NULL;
        len = 0;

        if (ret != EOK) {
            return ret;
        }

        /* Skip empty line. */
        if (trimmed == NULL) {
            continue;
        }

        /* Skip comments. */
        if (trimmed[0] == '#') {
            free(trimmed);
            continue;
        }

        *_line = trimmed;
        return EOK;
    }

    if (errno != 0) {
        return errno;
    }

    /* End of file. */
    return ENOENT;
}

static errno_t
authselect_read_conf_optional(FILE *file, char ***_optional)
{
    char **optional = NULL;
    char **reallocated;
    size_t count = 0;
    errno_t ret;

    do {
        count++;
        reallocated = realloc_array(optional, char *, count + 1);
        if (reallocated == NULL) {
            free_string_array(optional);
            return ENOMEM;
        }

        optional = reallocated;
        optional[count - 1] = NULL;
        optional[count] = NULL;

        ret = read_line(file, &optional[count - 1]);
    } while (ret == EOK);

    if (ret == ENOENT) {
        *_optional = optional;
        return EOK;
    }

    free_string_array(optional);
    return ret;
}

errno_t
authselect_read_conf(char **_profile_id,
                     char ***_optional)
{
    FILE *file;
    char *profile_id = NULL;
    char **optional = NULL;
    errno_t ret;

    file = fopen(PATH_CONFIG_FILE, "r");
    if (file == NULL) {
        ret = errno;

        if (ret == ENOENT) {
            WARN("Configuration file %s is missing", PATH_CONFIG_FILE);
        } else {
            ERROR("Unable to read configuration file %s", PATH_CONFIG_FILE);
        }

        return ret;
    }

    /* First line is a profile name. */
    ret = read_line(file, &profile_id);
    if (ret != EOK) {
        ERROR("Unable to read profile id from configuration [%d]: %s",
              ret, strerror(ret));
        goto done;
    }

    /* Following lines are options that were used. */
    ret = authselect_read_conf_optional(file, &optional);
    if (ret != EOK) {
        ERROR("Unable to read profile parameters from configuration [%d]: %s",
              ret, strerror(ret));
        goto done;
    }

    if (_profile_id != NULL) {
        *_profile_id = profile_id;
        profile_id = NULL;
    }

    if (_optional != NULL) {
        *_optional = optional;
        optional = NULL;
    }

done:
    fclose(file);

    if (profile_id != NULL) {
        free(profile_id);
    }

    free_string_array(optional);

    return ret;
}
