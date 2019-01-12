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
#include <errno.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "common/common.h"
#include "lib/constants.h"
#include "lib/util/util.h"
#include "lib/files/files.h"

#define RE_NSS "^\\s*([^[:space:]:]+):.*$"
#define RE_NSS_MATCHES 2

struct authselect_system_paths {
    const char *path;
    char **content;
};

struct authselect_system_templates {
    const char *template;
    char **generated;
};

errno_t
authselect_system_read_templates(const char *dirname,
                                 int dirfd,
                                 struct authselect_files **_templates)
{
    struct authselect_files *templates;
    errno_t ret;
    int i;

    templates = malloc_zero(struct authselect_files);
    if (templates == NULL) {
        return ENOMEM;
    }

    struct authselect_system_paths paths[] = {
        {FILE_SYSTEM,      &templates->systemauth},
        {FILE_PASSWORD,    &templates->passwordauth},
        {FILE_SMARTCARD,   &templates->smartcardauth},
        {FILE_FINGERPRINT, &templates->fingerprintauth},
        {FILE_POSTLOGIN,   &templates->postlogin},
        {FILE_NSSWITCH,    &templates->nsswitch},
        {FILE_DCONF_DB,    &templates->dconfdb},
        {FILE_DCONF_LOCK,  &templates->dconflock},
        {NULL, NULL},
    };

    for (i = 0; paths[i].path != NULL; i++) {
        INFO("Reading file [%s/%s]", dirname, paths[i].path);

        ret = textfile_read_dirfd(dirfd, dirname, paths[i].path,
                                  AUTHSELECT_FILE_SIZE_LIMIT,
                                  paths[i].content);
        if (ret == ENOENT) {
            *paths[i].content = NULL;
        } else if (ret != EOK) {
            ERROR("Unable to read file [%s/%s] [%d]: %s",
                  dirname, paths[i].path, ret, strerror(ret));
            authselect_files_free(templates);
            return ret;
        }
    }

    *_templates = templates;

    return EOK;
}

errno_t
authselect_system_nsswitch_find_maps(char *content,
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
authselect_system_nsswitch_delete_maps(char **maps,
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
                string_remove_line(match_string, m[1].rm_so);
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

static errno_t
authselect_system_generate_nsswitch(const char *template,
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
        ret = authselect_system_nsswitch_find_maps(generated, &maps);
        if (ret != EOK) {
            goto done;
        }

        ret = authselect_system_nsswitch_delete_maps(maps, user_content);
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

errno_t
authselect_system_generate(const char **features,
                           struct authselect_files *templates,
                           struct authselect_files **_files)
{
    struct authselect_files *files;
    errno_t ret;
    int i;

    if (templates == NULL) {
        return EINVAL;
    }

    files = malloc_zero(struct authselect_files);
    if (files == NULL) {
        return ENOMEM;
    }

    struct authselect_system_templates tpls[] = {
        {templates->systemauth,      &files->systemauth},
        {templates->passwordauth,    &files->passwordauth},
        {templates->smartcardauth,   &files->smartcardauth},
        {templates->fingerprintauth, &files->fingerprintauth},
        {templates->postlogin,       &files->postlogin},
        {templates->dconfdb,         &files->dconfdb},
        {templates->dconflock,       &files->dconflock},
        {NULL, NULL},
    };

    /* Template may be NULL so we must compare against destination. */
    for (i = 0; tpls[i].generated != NULL; i++) {
        if (tpls[i].template == NULL) {
            *tpls[i].generated = NULL;
            continue;
        }

        *tpls[i].generated = template_generate(tpls[i].template, features);
        if (tpls[i].generated == NULL) {
            ret = ENOMEM;
            goto done;
        }
    }

    /* nsswitch.conf is special as it can be merged with user-editable file */
    ret = authselect_system_generate_nsswitch(templates->nsswitch, features,
                                              &files->nsswitch);
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

static errno_t
authselect_system_write_temp(const char *path,
                             const char *content,
                             char **_tmp_file)
{
    errno_t ret;

    INFO("Writing temporary file for [%s]", path);
    ret = template_write_temporary(path, content, AUTHSELECT_FILE_MODE,
                                   _tmp_file);
    if (ret != EOK) {
        ERROR("Unable to write temporary file [%s] [%d]: %s",
              path, ret, strerror(ret));
        return ret;
    }

    INFO("Temporary file is named [%s]", *_tmp_file);

    return EOK;
}

static errno_t
authselect_system_rename_temp(char **tmp_path,
                              const char *final_path)
{
    errno_t ret;

    INFO("Renaming [%s] to [%s]", *tmp_path, final_path);

    ret = rename(*tmp_path, final_path);
    if (ret != 0) {
        ret = errno;
        ERROR("Unable to rename [%s] to [%s] [%d]: %s",
              *tmp_path, final_path, ret, strerror(ret));
    }

    free(*tmp_path);
    *tmp_path = NULL;

    return ret;
}

errno_t
authselect_system_write(const char **features,
                        struct authselect_files *templates)
{
    struct authselect_files *files;
    errno_t ret;
    int i;

    ret = authselect_system_generate(features, templates, &files);
    if (ret != EOK) {
        return ret;
    }

    struct authselect_generated generated[] = GENERATED_FILES(files);
    char *tmp_files[sizeof(generated)/sizeof(struct authselect_generated)] = {NULL};
    char *tmp_copies[sizeof(generated)/sizeof(struct authselect_generated)] = {NULL};

    /* First, write content into temporary files, so we can safely fail
     * on error. */
    for (i = 0; generated[i].path != NULL; i++) {
        ret = authselect_system_write_temp(generated[i].copy_path,
                                           generated[i].content,
                                           &tmp_copies[i]);
        if (ret != EOK) {
            goto done;
        }

        ret = authselect_system_write_temp(generated[i].path,
                                           generated[i].content,
                                           &tmp_files[i]);
        if (ret != EOK) {
            goto done;
        }
    }

    /* Now rename the files.
     *
     * We now know that the system is writable, so rename call shall not
     * fail and it will overwrite any existing file. The only reason it
     * can fail is EIO which we can not do anything about and we can not
     * even recover from it.
     */
    for (i = 0; generated[i].copy_path != NULL; i++) {
        ret = authselect_system_rename_temp(&tmp_copies[i],
                                            generated[i].copy_path);
        if (ret != EOK) {
            goto done;
        }
    }

    for (i = 0; generated[i].path != NULL; i++) {
        ret = authselect_system_rename_temp(&tmp_files[i], generated[i].path);
        if (ret != EOK) {
            goto done;
        }
    }

    ret = EOK;

done:
    if (ret != EOK) {
        for (i = 0; generated[i].path != NULL; i++) {
            if (tmp_copies[i] != NULL) {
                unlink(tmp_copies[i]);
                free(tmp_copies[i]);
                tmp_copies[i] = NULL;
            }

            if (tmp_files[i] != NULL) {
                unlink(tmp_files[i]);
                free(tmp_files[i]);
                tmp_files[i] = NULL;
            }
        }
    }
    authselect_files_free(files);

    return ret;
}

static bool
authselect_system_validate_file(const char *path,
                                const char *copy_path,
                                const char *expected)
{
    char *content;
    char *copy_content;
    errno_t ret;
    bool bret;

    INFO("Validating file [%s]", path);
    expected = expected == NULL ? "" : expected;

    ret = textfile_read(path, AUTHSELECT_FILE_SIZE_LIMIT, &content);
    if (ret == ENOENT) {
        ERROR("[%s] does not exist!", path);
        return false;
    } else if (ret == EACCES) {
        ERROR("Unable to read [%s] [%d]: %s", path, ret, strerror(ret));
        return false;
    } else if (ret != EOK) {
        ERROR("Unable to validate file [%s] [%d]: %s", path, ret, strerror(ret));
        return false;
    }

    ret = textfile_read(copy_path, AUTHSELECT_FILE_SIZE_LIMIT, &copy_content);
    if (ret == EOK) {
        /* Compare against copy of the originally generated files. */
        INFO("Comparing content against [%s]", copy_path);
        bret = strcmp(content, copy_content) == 0;
        free(copy_content);
    } else {
        INFO("Comparing content against current profile");
        bret = template_validate_written_content(content, expected);
    }

    free(content);
    if (!bret) {
        ERROR("[%s] has unexpected content!", path);
        return false;
    }

    ret = file_is_regular(path, AUTHSELECT_UID, AUTHSELECT_GID,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, &bret);
    if (ret != EOK) {
        ERROR("Unable to check file mode of [%s] [%d]: %s",
              path, ret, strerror(ret));
        return false;
    }

    return bret;
}

bool
authselect_system_validate(struct authselect_files *files)
{
    struct authselect_generated generated[] = GENERATED_FILES(files);
    bool result = true;
    bool bret;
    int i;

    for (i = 0; generated[i].path != NULL; i++) {
        bret = authselect_system_validate_file(generated[i].path,
                                               generated[i].copy_path,
                                               generated[i].content);
        result &= bret;
        if (!bret) {
            WARN("File [%s] was modified outside authselect!",
                 generated[i].path);
        }
    }

    return result;
}

bool
authselect_system_validate_missing()
{
    struct authselect_generated generated[] = GENERATED_FILES_PATHS;
    bool result = true;
    errno_t ret;
    int i;

    for (i = 0; generated[i].path != NULL; i++) {
        ret = file_exists(generated[i].path);
        if (ret == EOK) {
            ERROR("File [%s] is still present", generated[i].path);
            result = false;
            continue;
        } else if (ret != ENOENT) {
            ERROR("Error while trying to access file [%s] [%d]: %s",
                  generated[i].path, ret, strerror(ret));
            result = false;
        }
    }

    return result;
}

static errno_t
authselect_system_backup_create_dir_name(const char *name, char **_path)
{
    char *path;
    errno_t ret;

    path = format("%s/%s", AUTHSELECT_BACKUP_DIR, name);
    if (path == NULL) {
        return ENOMEM;
    }

    ret = file_make_path(path, AUTHSELECT_DIR_MODE);
    if (ret != EOK) {
        free(path);
        ERROR("Unable to create backup directory [%s/%s] [%d]: %s",
              AUTHSELECT_BACKUP_DIR, name, ret, strerror(ret));
        return ret;
    }

    *_path = path;

    return EOK;
}

static errno_t
authselect_system_backup_create_dir(const char *name, char **_path)
{
    struct tm *tm;
    char date[255];
    char *path;
    time_t now;
    size_t n;
    errno_t ret;

    if (name != NULL && name[0] != '\0') {
        return authselect_system_backup_create_dir_name(name, _path);
    }

    ret = file_make_path(AUTHSELECT_BACKUP_DIR, AUTHSELECT_DIR_MODE);
    if (ret != EOK) {
        ERROR("Unable to create backup directory [%s] [%d]: %s",
              AUTHSELECT_BACKUP_DIR, ret, strerror(ret));
        return ret;
    }

    now = time(NULL);
    tm = gmtime(&now);
    if (tm == NULL) {
        return EINVAL;
    }

    n = strftime(date, sizeof(date), "%Y-%m-%d-%H-%M-%S", tm);
    if (n == 0) {
        return EINVAL;
    }

    path = format("%s/%s.XXXXXX", AUTHSELECT_BACKUP_DIR, date);
    if (path == NULL) {
        return ENOMEM;
    }

    INFO("Creating temporary directory at [%s]", path);
    if (mkdtemp(path) == NULL) {
        ret = errno;
        free(path);
        return ret;
    }

    *_path = path;

    return EOK;
}

errno_t
authselect_system_backup(const char *backup_name, char **_path)
{
    struct authselect_symlink files[] = {SYMLINK_FILES};
    char *backup_path = NULL;
    const char *filename;
    errno_t ret;
    int i;

    ret = authselect_system_backup_create_dir(backup_name, &backup_path);
    if (ret != EOK) {
        ERROR("Unable to create backup directory [%d]: %s", ret, strerror(ret));
        return ret;
    }

    for (i = 0; files[i].name != NULL; i++) {
        filename = file_get_basename(files[i].name);
        if (filename == NULL) {
            ERROR("There is no filename in [%s]", files[i].name);
            ret = EINVAL;
            goto done;
        }

        INFO("Copying [%s] to [%s/%s]", files[i].name, backup_path, filename);
        ret = textfile_copy(files[i].name, backup_path, filename,
                            AUTHSELECT_DIR_MODE);
        if (ret == ENOENT) {
            WARN("File [%s] does not exist", files[i].name);
        } else if (ret != EOK) {
            ERROR("Unable to copy [%s] to [%s/%s] [%d]: %s", files[i].name,
                  backup_path, filename, ret, strerror(ret));
            goto done;
        }
    }

    *_path = backup_path;

    ret = EOK;

done:
    if (ret != EOK) {
        free(backup_path);
    }

    return ret;
}
