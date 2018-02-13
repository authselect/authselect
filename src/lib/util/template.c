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

#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "common/common.h"
#include "util/template.h"
#include "util/textfile.h"
#include "util/string.h"

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
process_condition_endfile(const char *chunk, const char **features)
{
    size_t option_len;
    int i;

    if (chunk[0] != '?' || chunk[1] != '?') {
        /* Not a conditional line. Return what we have. */
        return chunk;
    }

    if (features == NULL) {
        /* No options were specified, condition was not met. */
        return NULL;
    }

    /* The line contains at least "??" at the beginning, skip it. */
    chunk += 2;

    for (i = 0; features[i] != NULL; i++) {
        option_len = strlen(features[i]);
        if (strncmp(chunk, features[i], option_len) != 0) {
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
 * Include the next line only if condition is present in @features array.
 */
static const char *
process_condition_if_true(const char *chunk, const char **features)
{
    size_t option_len;
    int i;

    if (features == NULL) {
        /* No options where specified, condition was not met. */
        goto skip;
    }

    for (i = 0; features[i] != NULL; i++) {
        option_len = strlen(features[i]);
        if (strncmp(chunk, features[i], option_len) != 0) {
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
 * Include the next line only if condition is not present in @features array.
 */
static const char *
process_condition_if_false(const char *chunk, const char **features)
{
    size_t option_len;
    int i;

    if (features == NULL) {
        /* No options where specified, include next line. */
        return next_line(chunk);
    }

    for (i = 0; features[i] != NULL; i++) {
        option_len = strlen(features[i]);
        if (strncmp(chunk, features[i], option_len) != 0) {
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
process_condition_nextline(const char *chunk, const char **features)
{
    if (chunk[0] != '?') {
        /* Not a conditional line. */
        goto done;
    }

    switch (chunk[1]) {
    case '!':
        /* The line contains at least "?!" at the beginning, skip it. */
        return process_condition_if_false(chunk + 2, features);
    default:
        /* The line contains at least "?!" at the beginning, skip it. */
        return process_condition_if_true(chunk + 1, features);
    }

done:
    return chunk;
}

static void
process_chunk(const char *chunk,
              const char **features,
              const char **_line,
              const char **_remainder)
{
    const char *prev_chunk;

    chunk = process_condition_endfile(chunk, features);
    if (chunk == NULL) {
        goto done;
    }

    do {
        /* The obtained chunk may be a conditional line as well so we need
         * to process it in a loop. */
        prev_chunk = chunk;
        chunk = process_condition_nextline(chunk, features);
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

char *
template_generate(const char *template,
                  const char **features)
{
    const char *remainder;
    const char *line;
    char *output;

    if (template == NULL) {
        return strdup("");
    }

    output = malloc_zero_array(char, strlen(template) + 1);
    if (output == NULL) {
        return NULL;
    }

    /* Iterate over lines and add each line into the generated output
     * unless it is a conditional line which is not allowed in @features. */
    remainder = template;
    do {
        process_chunk(remainder, features, &line, &remainder);
        append_line(output, line);
    } while (remainder != NULL);

    return output;
}

static char *
template_generate_preamble()
{
    const char *timestr;
    char *preamble;
    char *trimmed;
    time_t now;

    now = time(NULL);
    timestr = ctime(&now);
    if (timestr == NULL) {
        ERROR("Unable to get current time!");
        return NULL;
    }

    trimmed = string_trim_noempty(timestr);
    if (trimmed == NULL) {
        return NULL;
    }

    preamble = format("# Generated by authselect on %s\n"
                      "# Do not modify this file manually.\n\n",
                      trimmed);
    free(trimmed);
    if (preamble == NULL) {
        ERROR("Unable to create message!");
        return NULL;
    }

    return preamble;
}

errno_t
template_write(const char *filepath,
               const char *content,
               mode_t mode)
{
    char *preamble;
    char *output;
    errno_t ret;

    preamble = template_generate_preamble();
    if (preamble == NULL) {
        return ENOMEM;
    }

    if (content == NULL) {
        output = preamble;
    } else {
        output = format("%s%s", preamble, content);
        free(preamble);
        if (output == NULL) {
            return ENOMEM;
        }
    }

    ret = textfile_write(filepath, output, mode);
    free(output);

    return ret;
}

bool
template_validate_written_content(const char *file_content,
                                  const char *expected)
{
    int i;

    /* Each generated file contains a "generated by" preamble that consists
     * of two comment lines followed by an empty line.
     *
     * We don't really care if someone has modified the preamble, so we
     * just check if the lines are still commented out.
     */

    for (i = 0; i < 2; i++) {
        if (file_content[0] != '#') {
            return false;
        }

        file_content = next_line(file_content);
        if (file_content == NULL) {
            return false;
        }
    }

    if (file_content[0] != '\n') {
        return false;
    }

    file_content++;

    return strcmp(file_content, expected) == 0;
}
