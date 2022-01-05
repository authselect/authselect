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
    ret = authselect_nsswitch_generate(templates->nsswitch, features,
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
                             time_t timestamp,
                             char **_tmp_file)
{
    errno_t ret;

    INFO("Writing temporary file for [%s]", path);
    ret = template_write_temporary(path, content, AUTHSELECT_FILE_MODE, timestamp,
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
    time_t now;
    int i;
    bool maintain_shadow = true;

    ret = should_maintain_shadow_copy(&maintain_shadow);
    if (ret != EOK) {
        return ret;
    }

    ret = authselect_system_generate(features, templates, &files);
    if (ret != EOK) {
        return ret;
    }

    struct authselect_generated generated[] = GENERATED_FILES(files);
    char *tmp_files[sizeof(generated)/sizeof(struct authselect_generated)] = {NULL};
    char *tmp_copies[sizeof(generated)/sizeof(struct authselect_generated)] = {NULL};

    /* First, write content into temporary files, so we can safely fail
     * on error. */
    now = time(NULL);
    for (i = 0; generated[i].path != NULL; i++) {
        if (maintain_shadow) {
            ret = authselect_system_write_temp(generated[i].copy_path,
                                               generated[i].content,
                                               now, &tmp_copies[i]);
            if (ret != EOK) {
                goto done;
            }
        }

        ret = authselect_system_write_temp(generated[i].path,
                                           generated[i].content,
                                           now, &tmp_files[i]);
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
    if (maintain_shadow) {
        for (i = 0; generated[i].copy_path != NULL; i++) {
            ret = authselect_system_rename_temp(&tmp_copies[i],
                                                generated[i].copy_path);
            if (ret != EOK) {
                goto done;
            }
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
