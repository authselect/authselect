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
#include <regex.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "common/common.h"
#include "util/template.h"
#include "util/textfile.h"
#include "util/string.h"
#include "util/string_array.h"

#define RE_MATCHES   9
#define RE_VALUE     "([^{}|]{0,})"
#define RE_FEATURE   "\"([^{}\"|]{1,})\""
#define OP_RE_LINE   "(continue if|stop if|include if|exclude if) " RE_FEATURE
#define OP_RE_IF     "(if|if not) " RE_FEATURE ":" RE_VALUE "(\\|" RE_VALUE "){0,1}"
#define OP_RE        "\\{(" OP_RE_LINE "|" OP_RE_IF ")\\}"

enum template_operator {
    OP_CONTINUE,
    OP_STOP,
    OP_INCLUDE,
    OP_EXCLUDE,
    OP_IF,
    OP_IF_NOT,

    OP_SENTINEL
};

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

static enum template_operator
template_match_get_operator(const char *match_string,
                            regmatch_t *m)
{
    size_t len;
    int i;
    struct {
        const char *str;
        enum template_operator op;
    } operators[] = {
        {"continue if", OP_CONTINUE},
        {"stop if", OP_STOP},
        {"include if", OP_INCLUDE},
        {"exclude if", OP_EXCLUDE},
        {"if not", OP_IF_NOT},
        {"if", OP_IF},
        {NULL, OP_SENTINEL}
    };

    for (i = 0; operators[i].str != NULL; i++) {
        len = strlen(operators[i].str);
        if (strncmp(match_string + m->rm_so, operators[i].str, len) == 0) {
            return operators[i].op;
        }
    }

    return OP_SENTINEL;
}

static char *
template_match_get_string(const char *match_string,
                          regmatch_t *match)
{
    if (match->rm_so == -1) {
        return strdup("");
    }

    return strndup(match_string + match->rm_so, match->rm_eo - match->rm_so);
}

static errno_t
template_match_get_feature(const char *match_string,
                           enum template_operator op,
                           regmatch_t *matches,
                           char **_feature)
{
    regmatch_t *match = NULL;
    char *feature;

    switch (op) {
    case OP_CONTINUE:
    case OP_STOP:
    case OP_INCLUDE:
    case OP_EXCLUDE:
        match = &matches[3];
        break;
    case OP_IF:
    case OP_IF_NOT:
        match = &matches[5];
        break;
    case OP_SENTINEL:
        ERROR("Invalid operator!");
        return EINVAL;
    }

    feature = template_match_get_string(match_string, match);
    if (feature == NULL) {
        return ENOMEM;
    }

    *_feature = feature;

    return EOK;
}

static errno_t
template_match_get_values(const char *match_string,
                          enum template_operator op,
                          regmatch_t *matches,
                          char **_if_true,
                          char **_if_false)
{
    regmatch_t *m_true;
    regmatch_t *m_false;
    char *if_true;
    char *if_false;

    switch (op) {
    case OP_CONTINUE:
    case OP_STOP:
    case OP_INCLUDE:
    case OP_EXCLUDE:
        *_if_true = NULL;
        *_if_false = NULL;
        return EOK;
    case OP_IF:
    case OP_IF_NOT:
        m_true = &matches[6];
        m_false = &matches[8];
        break;
    case OP_SENTINEL:
        ERROR("Invalid operator!");
        return EINVAL;
    }

    if_true = template_match_get_string(match_string, m_true);
    if (if_true == NULL) {
        return ENOMEM;
    }

    if_false = template_match_get_string(match_string, m_false);
    if (if_false == NULL) {
        free(if_true);
        return ENOMEM;
    }

    *_if_true = if_true;
    *_if_false = if_false;

    return EOK;
}

static errno_t
template_match_replace(const char **features,
                       char *match_string,
                       regmatch_t *match,
                       enum template_operator op,
                       const char *feature,
                       const char *if_true,
                       const char *if_false)
{
    const char *value;
    bool enabled;

    enabled = string_array_has_value((char **)features, feature);

    switch (op) {
    case OP_CONTINUE:
        if (enabled) {
            string_remove_line(match_string, match->rm_so);
            break;
        }

        string_remove_remainder(match_string, match->rm_so);
        break;
    case OP_STOP:
        if (!enabled) {
            string_remove_line(match_string, match->rm_so);
            break;
        }

        string_remove_remainder(match_string, match->rm_so);
        break;
    case OP_INCLUDE:
        if (enabled) {
            string_remove_range(match_string, match->rm_so, match->rm_eo);
            break;
        }

        string_remove_line(match_string, match->rm_so);
        break;
    case OP_EXCLUDE:
        if (!enabled) {
            string_remove_range(match_string, match->rm_so, match->rm_eo);
            break;
        }

        string_remove_line(match_string, match->rm_so);
        break;
    case OP_IF:
        value = enabled ? if_true : if_false;
        string_replace_position(match_string, match->rm_so, match->rm_eo, value);
        break;
    case OP_IF_NOT:
        value = !enabled ? if_true : if_false;
        string_replace_position(match_string, match->rm_so, match->rm_eo, value);
        break;
    case OP_SENTINEL:
        ERROR("Invalid operator!");
        return EINVAL;
    }

    return EOK;
}

/**
 * Supported operators:
 *
 * {continue if "with-smartcard"}
 * {stop if "with-smartcard"}
 * {exclude if "with-smartcard"}
 * {include if "with-smartcard"}
 * {if "with-smartcard":true|false}
 * {if "with-smartcard":true}
 * {if not "with-smartcard":true}
 * {if not "with-smartcard":true|false}
 *
 * Match groups for template regular expression are as follows:
 *
 * Match 0: {continue if "with-smartcard"}
 * Match 1: continue if "with-smartcard"
 * Match 2: continue if
 * Match 3: with-smartcard
 *
 * Match 0: {stop if "with-smartcard"}
 * Match 1: stop if "with-smartcard"
 * Match 2: stop if
 * Match 3: with-smartcard
 *
 * Match 0: {exclude if "with-smartcard"}
 * Match 1: exclude if "with-smartcard"
 * Match 2: exclude if
 * Match 3: with-smartcard
 *
 * Match 0: {include if "with-smartcard"}
 * Match 1: include if "with-smartcard"
 * Match 2: include if
 * Match 3: with-smartcard
 *
 * Match 0: {if "with-smartcard":true|false}
 * Match 1: if "with-smartcard":true|false
 * Match 4: if
 * Match 5: with-smartcard
 * Match 6: true
 * Match 7: |false
 * Match 8: false
 *
 * Match 0: {if "with-smartcard":true}
 * Match 1: if "with-smartcard":true
 * Match 4: if
 * Match 5: with-smartcard
 * Match 6: true
 *
 * Match 0: {if not "with-smartcard":true}
 * Match 1: if not "with-smartcard":true
 * Match 4: if not
 * Match 5: with-smartcard
 * Match 6: true
 *
 * Match 0: {if not "with-smartcard":true|false}
 * Match 1: if not "with-smartcard":true|false
 * Match 4: if not
 * Match 5: with-smartcard
 * Match 6: true
 * Match 7: |false
 * Match 8: false
 */
static errno_t
template_process_matches(const char *match_string,
                         regmatch_t *m,
                         enum template_operator *_op,
                         char **_feature,
                         char **_if_true,
                         char **_if_false)
{
    enum template_operator op;
    char *if_false;
    char *if_true;
    char *feature;
    errno_t ret;

    op = template_match_get_operator(match_string, &m[1]);
    if (op == OP_SENTINEL) {
        return EINVAL;
    }

    ret = template_match_get_feature(match_string, op, m, &feature);
    if (ret != EOK) {
        return ret;
    }

    ret = template_match_get_values(match_string, op, m, &if_true, &if_false);
    if (ret != EOK) {
        free(feature);
        return ret;
    }

    *_op = op;
    *_feature = feature;
    *_if_true = if_true;
    *_if_false = if_false;

    return EOK;
}

static errno_t
template_process_operators(const char **features,
                           char *content)
{

    regex_t regex;
    char *match_string;
    size_t orig_len;
    regmatch_t m[RE_MATCHES];
    enum template_operator op;
    char *if_false = NULL;
    char *if_true = NULL;
    char *feature = NULL;
    errno_t ret;
    int reret;

    orig_len = strlen(content);

    reret = regcomp(&regex, OP_RE, REG_EXTENDED | REG_NEWLINE);
    if (reret != REG_NOERROR) {
        ERROR("Unable to compile regular expression: regex error %d", reret);
        return EFAULT;
    }

    match_string = content;
    while ((reret = regexec(&regex, match_string, RE_MATCHES, m, 0)) == REG_NOERROR) {
        ret = template_process_matches(match_string, m, &op, &feature,
                                       &if_true, &if_false);
        if (ret != EOK) {
            ERROR("Unable to process match [%d]: %s", ret, strerror(ret));
            goto done;
        }

        ret = template_match_replace(features, match_string, &m[0], op,
                                     feature, if_true, if_false);

        if (feature != NULL) {
            free(feature);
        }

        if (if_true != NULL) {
            free(if_true);
        }

        if (if_false != NULL) {
            free(if_false);
        }

        if (ret != EOK) {
            ERROR("Unable to process operator [%d]: %s", ret, strerror(ret));
            goto done;
        }

        /* Since the whole line could have been removed, we have to find first
         * non-zero position. */
        match_string += m[0].rm_eo;
        while (*match_string == '\0' && match_string - content < orig_len) {
            match_string++;
        }
    }

    string_replace_shake(content, orig_len);

    if (reret != REG_NOMATCH) {
        ERROR("Unable to search string: regex error %d", reret);
        ret = EFAULT;
        goto done;
    }

    ret = EOK;

done:
    regfree(&regex);
    return ret;
}

char *
template_generate(const char *template,
                  const char **features)
{
    char **lines;
    char *output;
    errno_t ret;

    if (template == NULL) {
        return strdup("");
    }

    output = strdup(template);
    if (output == NULL) {
        return NULL;
    }

    ret = template_process_operators(features, output);
    if (ret != EOK) {
        ERROR("Unable to generate template [%d]: %s", ret, strerror(ret));
        free(output);
        return NULL;
    }

    /* Trim lines */
    lines = string_explode(output, '\n', STRING_EXPLODE_TRIM_RIGHT);
    free(output);
    if (lines == NULL) {
        return NULL;
    }

    output = string_implode((const char **)lines, '\n');
    string_array_free(lines);

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
