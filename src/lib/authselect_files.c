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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "authselect.h"
#include "lib/authselect_private.h"

static const char *
next_line(const char *chunk)
{
    const char *lineend;

    if (chunk == NULL) {
        return NULL;
    }

    lineend = strchr(chunk, '\n');

    return lineend == NULL ? NULL : lineend + 1;
}

/**
 * Condition format:
 * ??condition
 * file content
 *
 * If the condition is not met, stop file processing.
 */
static const char *
process_condition_endfile(const char *chunk, const char **optional)
{
    size_t option_len;
    int i;

    if (chunk[0] != '?' || chunk[1] != '?') {
        /* Not a conditional line. Return what we have. */
        return chunk;
    }

    if (optional == NULL) {
        /* No options were specified, condition was not met. */
        return NULL;
    }

    /* The line contains at least "??" at the beginning, skip it. */
    chunk += 2;

    for (i = 0; optional[i] != NULL; i++) {
        option_len = strlen(optional[i]);
        if (strncmp(chunk, optional[i], option_len) != 0) {
            continue;
        }

        /* We have a match, now we must check that the character behind
         * option name is a whitespace or tab so we can avoid
         * overlapping names. */
        if (!isspace(chunk[option_len])) {
            continue;
        }

        /* Condition was met, continue file processing on next line. */
        return next_line(chunk);
    }

    /* Condition was not met, stop file processing. */
    return NULL;
}

/**
 * Condition: ?condition:
 *
 * Include the next line only if condition is present in @optional array.
 */
static const char *
process_condition_if_true(const char *chunk, const char **optional)
{
    size_t option_len;
    int i;

    if (optional == NULL) {
        /* No options where specified, condition was not met. */
        goto skip;
    }

    for (i = 0; optional[i] != NULL; i++) {
        option_len = strlen(optional[i]);
        if (strncmp(chunk, optional[i], option_len) != 0) {
            continue;
        }

        /* We have a match, now we must check that the character behind
         * option name is a colon so we can avoid overlapping names. */
        if (chunk[option_len] != ':') {
            continue;
        }

        /* Condition was met, continue file processing on next line. */
        return next_line(chunk);
    }

skip:
    /* Condition was not met, skip this and the next line
     * and continue file processing. */
    return next_line(next_line(chunk));
}

/**
 * Condition: ?!condition:
 *
 * Include the next line only if condition is not present in @optional array.
 */
static const char *
process_condition_if_false(const char *chunk, const char **optional)
{
    size_t option_len;
    int i;

    if (optional == NULL) {
        /* No options where specified, include next line. */
        return next_line(chunk);
    }

    for (i = 0; optional[i] != NULL; i++) {
        option_len = strlen(optional[i]);
        if (strncmp(chunk, optional[i], option_len) != 0) {
            continue;
        }

        /* We have a match, now we must check that the character behind
         * option name is a colon so we can avoid overlapping names. */
        if (chunk[option_len] != ':') {
            continue;
        }

        /* Option is present. Exclude this and next line. */
        return next_line(next_line(chunk));
    }

    /* Option is not present. Include next line. */
    return next_line(chunk);
}

/**
 * Condition format:
 * ?condition:
 * line content if condition is met
 *
 * ?!condition:
 * line content if condition is not ment
 */
static const char *
process_condition_nextline(const char *chunk, const char **optional)
{
    if (chunk[0] != '?') {
        /* Not a conditional line. */
        goto done;
    }

    switch (chunk[1]) {
    case '!':
        /* The line contains at least "?!" at the beginning, skip it. */
        return process_condition_if_false(chunk + 2, optional);
    default:
        /* The line contains at least "?!" at the beginning, skip it. */
        return process_condition_if_true(chunk + 1, optional);
    }

done:
    return chunk;
}

static void
process_chunk(const char *chunk,
              const char **optional,
              const char **_line,
              const char **_remainder)
{
    const char *prev_chunk;

    chunk = process_condition_endfile(chunk, optional);
    if (chunk == NULL) {
        goto done;
    }

    do {
        /* The obtained chunk may be a conditional line as well so we need
         * to process it in a loop. */
        prev_chunk = chunk;
        chunk = process_condition_nextline(chunk, optional);
    } while (chunk != NULL && prev_chunk != chunk);

done:
    *_line = chunk;
    *_remainder = next_line(chunk);

    return;
}

static void
append_line(char *destination, const char *line)
{
    const char *lineend;
    size_t len;

    if (line == NULL) {
        return;
    }

    lineend = strchr(line, '\n');

    len = lineend == NULL ? strlen(line) : lineend + 1 - line;

    strncat(destination, line, len);
}

static errno_t
generate_file(const char *template,
              const char **optional,
              char **_generated)
{
    const char *remainder;
    const char *line;
    char *output;

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
    remainder = template;
    do {
        process_chunk(remainder, optional, &line, &remainder);
        append_line(output, line);
    } while (remainder != NULL);

    /* If the generated output is empty we consider it as
     * the file does not exist so we return NULL. */
    if (output[0] == '\0') {
        free(output);
        output = NULL;
    }

    *_generated = output;

    return EOK;
}

static char *
generate_dconf_option(char *content,
                      const char *option,
                      const char *option_file)
{
    const char *value;
    char *line;

    value = option_file == NULL || option_file[0] == '\0' ? "false" : "true";

    line = format("%s=%s", option, value);
    if (line == NULL) {
        free(content);
        return NULL;
    }

    content = buffer_append_line(content, line);
    free(line);

    return content;
}

static errno_t
generate_dconf_db(struct authselect_files *files)
{
    char *content = NULL;

    content = buffer_append_line(NULL, "[org/gnome/login-screen]");
    if (content == NULL) {
        return ENOMEM;
    }

    content = generate_dconf_option(content,
                                    "enable-smartcard-authentication",
                                    files->smartcardauth);
    if (content == NULL) {
        return ENOMEM;
    }

    content = generate_dconf_option(content,
                                    "enable-fingerprint-authentication",
                                    files->smartcardauth);
    if (content == NULL) {
        return ENOMEM;
    }

    files->dconfdb = content;

    return EOK;
}

static errno_t
generate_dconf_lock(struct authselect_files *files)
{
    char *dup;
    static const char *lock = \
        "/org/gnome/login-screen/enable-smartcard-authentication\n"
        "/org/gnome/login-screen/enable-fingerprint-authentication\n";


    /* We strdup the content here so we do not special case it from
     * other files content. */

    dup = strdup(lock);
    if (dup == NULL) {
        return ENOMEM;
    }

    files->dconflock = dup;

    return EOK;

}

static errno_t
generate_dconf(struct authselect_files *files)
{
    errno_t ret;

    ret = generate_dconf_db(files);
    if (ret != EOK) {
        ERROR("Unable to generate dconf db file [%d]: %s", ret, strerror(ret));
        return ret;
    }

    ret = generate_dconf_lock(files);
    if (ret != EOK) {
        ERROR("Unable to generate dconf lock file [%d]: %s", ret, strerror(ret));
        return ret;
    }

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
        {profile->files.postlogin,       &files->postlogin},
        {profile->files.nsswitch,        &files->nsswitch},
        {NULL, NULL},
    };

    /* Template may be NULL so we must compare against destination. */
    for (i = 0; tpls[i]._storage != NULL; i++) {
        ret = generate_file(tpls[i].template, optional, tpls[i]._storage);
        if (ret != EOK) {
            goto done;
        }
    }

    /* Generate dconf db and lock content. */
    ret = generate_dconf(files);
    if (ret != EOK) {
        goto done;
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

_PUBLIC_ const char *
authselect_files_postlogin(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->postlogin;
}

_PUBLIC_ const char *
authselect_files_dconf_db(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->dconfdb;
}

_PUBLIC_ const char *
authselect_files_dconf_lock(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->dconflock;
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

    if (files->postlogin != NULL) {
        free(files->postlogin);
    }

    if (files->nsswitch != NULL) {
        free(files->nsswitch);
    }

    if (files->dconfdb != NULL) {
        free(files->dconfdb);
    }

    if (files->dconflock != NULL) {
        free(files->dconflock);
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
