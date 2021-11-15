/*
    Authors:
        Pavel Březina <pbrezina@redhat.com>

    Copyright (C) 2021 Red Hat

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
#include <regex.h>
#include <string.h>
#include <stdlib.h>

#include "common/common.h"
#include "lib/constants.h"
#include "lib/util/util.h"
#include "lib/files/files.h"

#define RE_NSS "^\\s*([^[:space:]:]+):.*$"
#define RE_NSS_MATCHES 2

errno_t
authselect_nsswitch_find_maps(char *content,
                              char ***_maps)
{
    char *match_string;
    regmatch_t m[RE_NSS_MATCHES];
    regex_t regex;
    errno_t ret;
    char **maps;
    int reret;

    maps = string_array_create(10);
    if (maps == NULL) {
        return ENOMEM;
    }

    reret = regcomp(&regex, RE_NSS, REG_EXTENDED | REG_NEWLINE);
    if (reret != REG_NOERROR) {
        ERROR("Unable to compile regular expression: regex error %d", reret);
        ret = EFAULT;
        goto done;
    }

    match_string = content;
    while ((reret = regexec(&regex, match_string, 2, m, 0)) == REG_NOERROR) {
        maps = string_array_add_value_safe(maps, match_string + m[1].rm_so,
                                           m[1].rm_eo - m[1].rm_so, true);
        if (maps == NULL) {
            ret = ENOMEM;
            goto done;
        }

        match_string += m[0].rm_eo;
    }

    if (reret != REG_NOMATCH) {
        ERROR("Unable to search string: regex error %d", reret);
        ret = EFAULT;
        goto done;
    }

    *_maps = maps;

    ret = EOK;

done:
    regfree(&regex);
    if (ret != EOK) {
        string_array_free(maps);
    }

    return ret;
}

static errno_t
authselect_nsswitch_delete_maps(char **maps,
                                char *content)
{
    char *match_string;
    const char *map_name;
    size_t map_len;
    size_t orig_len;
    regmatch_t m[RE_NSS_MATCHES];
    regex_t regex;
    errno_t ret;
    int reret;
    int i;

    if (string_is_empty(content)) {
        return EOK;
    }

    orig_len = strlen(content);

    reret = regcomp(&regex, RE_NSS, REG_EXTENDED | REG_NEWLINE);
    if (reret != REG_NOERROR) {
        ERROR("Unable to compile regular expression: regex error %d", reret);
        ret = EFAULT;
        goto done;
    }

    match_string = content;
    while ((reret = regexec(&regex, match_string, 2, m, 0)) == REG_NOERROR) {
        map_name = match_string + m[1].rm_so;
        map_len = m[1].rm_eo - m[1].rm_so;
        for (i = 0; maps[i] != NULL; i++) {
            if (strncmp(map_name, maps[i], map_len) == 0) {
                string_remove_line(content, match_string, m[1].rm_so);
                break;
            }
        }

        /* Since the whole line could have been removed, we have to find first
         * non-zero position. */
        match_string += m[0].rm_eo;
        while (*match_string == '\0' && match_string - content < orig_len) {
            match_string++;
        }
    }

    if (reret != REG_NOMATCH) {
        ERROR("Unable to search string: regex error %d", reret);
        ret = EFAULT;
        goto done;
    }

    string_replace_shake(content, orig_len);

    ret = EOK;

done:
    regfree(&regex);

    return ret;
}

errno_t
authselect_nsswitch_generate(const char *template,
                             const char **features,
                             char **_content)
{
    static const char *preambule = \
    "# If you want to make changes to nsswitch.conf please modify\n"
    "# " PATH_USER_NSSWITCH " and run 'authselect apply-changes'.\n"
    "#\n"
    "# Note that your changes may not be applied as they may be\n"
    "# overwritten by selected profile. Maps set in the authselect\n"
    "# profile takes always precedence and overwrites the same maps\n"
    "# set in the user file. Only maps that are not set by the profile\n"
    "# are applied from the user file.\n"
    "#\n"
    "# For example, if the profile sets:\n"
    "#     passwd: sss files\n"
    "# and " PATH_USER_NSSWITCH " contains:\n"
    "#     passwd: files\n"
    "#     hosts: files dns\n"
    "# the resulting generated nsswitch.conf will be:\n"
    "#     passwd: sss files # from profile\n"
    "#     hosts: files dns  # from user file\n\n";
    char *user_content = NULL;
    char *generated = NULL;
    char *content = NULL;
    char **maps = NULL;
    errno_t ret;

    generated = template_generate(template, features);
    if (generated == NULL) {
        ret = ENOMEM;
        goto done;
    }

    ret = textfile_read(PATH_USER_NSSWITCH, AUTHSELECT_FILE_SIZE_LIMIT,
                        &user_content);
    switch (ret) {
    case EOK:
        ret = authselect_nsswitch_find_maps(generated, &maps);
        if (ret != EOK) {
            goto done;
        }

        ret = authselect_nsswitch_delete_maps(maps, user_content);
        if (ret != EOK) {
            goto done;
        }

        if (string_is_empty(user_content)) {
            content = format("%s%s", preambule, generated);
            break;
        }

        content = format("%s%s\n# Included from %s\n\n%s",
                         preambule, generated, PATH_USER_NSSWITCH,
                         user_content);
        break;
    case ENOENT:
        content = format("%s%s", preambule, generated);
        break;
    default:
        ERROR("Unable to read [%s] [%d]: %s", PATH_USER_NSSWITCH,
              ret, strerror(ret));
        goto done;
    }

    if (content == NULL) {
        ret = ENOMEM;
        goto done;
    }

    *_content = content;

    ret = EOK;

done:
    if (ret != EOK) {
        ERROR("Unable to generate nsswitch.conf [%d]: %s", ret, strerror(ret));
    }

    free(user_content);
    free(generated);
    string_array_free(maps);

    return ret;
}
