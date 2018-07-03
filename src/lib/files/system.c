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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "common/common.h"
#include "lib/constants.h"
#include "lib/util/util.h"
#include "lib/files/files.h"

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
        {templates->nsswitch,        &files->nsswitch},
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

    *_files = files;

    ret = EOK;

done:
    if (ret != EOK) {
        ERROR("Unable to generate files [%d]: %s", ret, strerror(ret));
        authselect_files_free(files);
    }

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
    char *tmpfiles[sizeof(generated)/sizeof(struct authselect_generated)];

    /* First, write content into temporary files, so we can safely fail
     * on error. */
    for (i = 0; generated[i].path != NULL; i++) {
        INFO("Writing temporary file for [%s]", generated[i].path);
        ret = template_write_temporary(generated[i].path, generated[i].content,
                             AUTHSELECT_FILE_MODE, &tmpfiles[i]);
        if (ret != EOK) {
            goto done;
        }
        INFO("Temporary file is named [%s]", tmpfiles[i]);
    }

    /* Now rename the files. */
    for (i = 0; generated[i].path != NULL; i++) {
        INFO("Renaming [%s] to [%s]", tmpfiles[i], generated[i].path);
        /* We now know that the system is writable, so rename call shall not
         * fail and it will overwrite any existing file. The only reason it
         * can fail is EIO which we can not do anything about and we can not
         * even recover from it. */
        ret = rename(tmpfiles[i], generated[i].path);
        if (ret != 0) {
            ret = errno;
            ERROR("Unable to rename [%s] to [%s] [%d]: %s",
                  tmpfiles[i], generated[i].path, ret, strerror(ret));
            goto done;
        }

        free(tmpfiles[i]);
        tmpfiles[i] = NULL;
    }

    ret = EOK;

done:
    if (ret != EOK) {
        for (i = 0; generated[i].path != NULL; i++) {
            if (tmpfiles[i] != NULL) {
                unlink(tmpfiles[i]);
                free(tmpfiles[i]);
                tmpfiles[i] = NULL;
            }
        }
    }
    authselect_files_free(files);

    return ret;
}

static bool
authselect_system_validate_file(const char *path,
                                const char *expected)
{
    char *content;
    errno_t ret;
    bool bret;

    INFO("Validating file [%s]", path);

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

    if (expected == NULL) {
        expected = "";
    }

    bret = template_validate_written_content(content, expected);
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
    char *filename;
    errno_t ret;
    int i;

    ret = authselect_system_backup_create_dir(backup_name, &backup_path);
    if (ret != EOK) {
        ERROR("Unable to create backup directory [%d]: %s", ret, strerror(ret));
        return ret;
    }

    for (i = 0; files[i].name != NULL; i++) {
        filename = strrchr(files[i].name, '/');
        if (filename == NULL || filename[0] == '\0' || filename[1] == '\0') {
            ERROR("There is no filename in [%s]", files[i].name);
            ret = EINVAL;
            goto done;
        }
        filename++;

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
