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

#include "config.h"

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

errno_t
authselect_nsswitch_generate(const char *template,
                             const char **features,
                             char **_content)
{
    char *content;

    content = template_generate(template, features);
    if (content == NULL) {
        return ENOMEM;
    }

    *_content = content;

    return EOK;
}
