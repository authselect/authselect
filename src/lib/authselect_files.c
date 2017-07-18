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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "authselect.h"
#include "authselect_private.h"

static const char *
line_to_include(const char *line, const char **optional)
{
    size_t option_len;
    int i;

    if (line[0] != '?') {
        /* Not a conditional line. */
        return line;
    }

    if (optional == NULL) {
        /* No options where specified, do not include this line. */
        return NULL;
    }

    line++;

    for (i = 0; optional[i] != NULL; i++) {
        option_len = strlen(optional[i]);
        if (strncmp(line, optional[i], option_len) != 0) {
            continue;
        }

        /* We have a much, now we must check that the character behind
         * option name is a space or tab so we avoid overlapping names. */
        if (line[option_len] != ' ' && line[option_len] != '\t') {
            continue;
        }

        /* Include this line without the option part. */
        return line + option_len + 1;
    }

    /* This option was not specified, do not include this line. */
    return NULL;
}

static void
append_line(char *destination, const char *line, int len)
{
    if (line == NULL) {
        return;
    }

    if (len < 0) {
        len = strlen(line);
    }

    strncat(destination, line, len);
}

static errno_t
generate_file(const char *template,
              const char **optional,
              char **_generated)
{
    const char *chunk;
    const char *line;
    char *lineend;
    char *output;
    int len;

    if (template == NULL) {
        *_generated = NULL;
        return EOK;
    }

    output = malloc_zero_array(char, strlen(template) + 1);
    if (output == NULL) {
        return ENOMEM;
    }

    /* Iterate over lines and add each line into the generated output
     * unless it is a conditional line which is not allowed in @optional. */
    chunk = template;
    do {
        line = line_to_include(chunk, optional);
        lineend = strchr(chunk, '\n');
        len = lineend == NULL ? -1 : lineend - line + 1;

        append_line(output, line, len);
        chunk = lineend + 1;
    } while (lineend != NULL);

    *_generated = output;

    return EOK;
}

errno_t
authselect_files_generate(struct authselect_profile *profile,
                          const char **optional,
                          struct authselect_files **_files)
{
    struct authselect_files *files;
    errno_t ret;
    int i;

    if (profile == NULL) {
        return EINVAL;
    }

    files = malloc_zero(struct authselect_files);
    if (files == NULL) {
        return ENOMEM;
    }

    struct {
        const char *template;
        char **_storage;
    } tpls[] = {
        {profile->files.systemauth,      &files->systemauth},
        {profile->files.passwordauth,    &files->passwordauth},
        {profile->files.smartcardauth,   &files->smartcardauth},
        {profile->files.fingerprintauth, &files->fingerprintauth},
        {profile->files.nsswitch,        &files->nsswitch},
        {NULL, NULL},
    };

    for (i = 0; tpls[i]._storage != NULL; i++) {
        ret = generate_file(tpls[i].template, optional, tpls[i]._storage);
        if (ret != EOK) {
            goto done;
        }
    }

    *_files = files;

    ret = EOK;

done:
    if (ret != EOK) {
        ERROR("Unable to generate files [%d]: %s", ret, strerror(ret));
        authselect_files_free(files);
    }

    return ret;
}

_PUBLIC_ const char *
authselect_files_nsswitch(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->nsswitch;
}

_PUBLIC_ const char *
authselect_files_systemauth(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->systemauth;
}

_PUBLIC_ const char *
authselect_files_passwordauth(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->passwordauth;
}

_PUBLIC_ const char *
authselect_files_smartcardauth(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->smartcardauth;
}

_PUBLIC_ const char *
authselect_files_fingerprintauth(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->fingerprintauth;
}

void
authselect_files_free_content(struct authselect_files *files)
{
    if (files == NULL) {
        return;
    }

    if (files->systemauth != NULL) {
        free(files->systemauth);
    }

    if (files->passwordauth != NULL) {
        free(files->passwordauth);
    }

    if (files->smartcardauth != NULL) {
        free(files->smartcardauth);
    }

    if (files->fingerprintauth != NULL) {
        free(files->fingerprintauth);
    }

    if (files->nsswitch != NULL) {
        free(files->nsswitch);
    }
}

_PUBLIC_ void
authselect_files_free(struct authselect_files *files)
{
    if (files == NULL) {
        return;
    }

    authselect_files_free_content(files);

    free(files);
}
