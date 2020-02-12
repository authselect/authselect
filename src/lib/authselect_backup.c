/*
    Authors:
        Pavel Březina <pbrezina@redhat.com>

    Copyright (C) 2019 Red Hat

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
#include <string.h>
#include <stdlib.h>

#include "authselect.h"
#include "common/common.h"
#include "lib/constants.h"
#include "lib/util/util.h"
#include "lib/files/files.h"
#include "lib/profiles/profiles.h"

static errno_t
authselect_backup_create_named(const char *name,
                               char **_path)
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
authselect_backup_create_anonymous(char **_path)
{
    struct tm *tm;
    char date[255];
    char *path;
    time_t now;
    size_t n;
    errno_t ret;

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

static errno_t
authselect_backup_create(const char *name, char **_path)
{
    if (name != NULL && name[0] != '\0') {
        return authselect_backup_create_named(name, _path);
    }

    return authselect_backup_create_anonymous(_path);
}

static errno_t
authselect_backup_system_configuration(const char *path)
{
    struct authselect_symlink files[] = {SYMLINK_FILES};
    const char *filename;
    errno_t ret;
    int i;

    for (i = 0; files[i].name != NULL; i++) {
        filename = file_get_basename(files[i].dest);
        if (filename == NULL) {
            ERROR("There is no filename in [%s]", files[i].dest);
            return EINVAL;
        }

        INFO("Copying [%s] to [%s/%s]", files[i].name, path, filename);
        ret = selinux_file_copy(files[i].name, path, filename,
                                AUTHSELECT_DIR_MODE);
        if (ret == ENOENT) {
            WARN("File [%s] does not exist", files[i].name);
        } else if (ret != EOK) {
            ERROR("Unable to copy [%s] to [%s/%s] [%d]: %s", files[i].name,
                  path, filename, ret, strerror(ret));
            return ret;
        }
    }

    return EOK;
}

static errno_t
authselect_backup_authselect_configuration(const char *path)
{
    errno_t ret;

    ret = selinux_file_copy(PATH_CONFIG_FILE, path, FILE_CONFIG,
                            AUTHSELECT_DIR_MODE);
    if (ret != EOK) {
        ERROR("Unable to copy [%s] to [%s/%s] [%d]: %s", PATH_CONFIG_FILE,
              path, FILE_CONFIG, ret, strerror(ret));
        return ret;
    }

    return authselect_backup_system_configuration(path);
}

_PUBLIC_ int
authselect_backup(const char *name, char **_path)
{
    char *path = NULL;
    bool is_valid;
    errno_t ret;

    ret = authselect_backup_create(name, &path);
    if (ret != EOK) {
        goto done;
    }

    ret = authselect_validate_configuration(&is_valid);
    if (ret == EOK && is_valid) {
        /* Valid authselect configuration. */
        INFO("Trying to backup authselect configuration to [%s]", path);
        ret = authselect_backup_authselect_configuration(path);
        goto done;
    }

    INFO("Trying to backup system configuration to [%s]", path);
    ret = authselect_backup_system_configuration(path);

done:
    if (ret == EOK) {
        INFO("Backup was successfully created at [%s]", path);
        *_path = path;
        ret = EOK;
    } else if (ret != EOK) {
        ERROR("Unable to create backup [%d]: %s", ret, strerror(ret));
        free(path);
    }

    return ret;
}

_PUBLIC_ char **
authselect_backup_list(void)
{
    char **names;
    errno_t ret;

    ret = dir_list(AUTHSELECT_BACKUP_DIR,
                   DIR_LIST_DIRS | DIR_LIST_SORT_BY_CTIME,
                   &names, NULL);
    if (ret != EOK) {
        ERROR("Unable to list directory [%s] [%d]: %s",
              AUTHSELECT_BACKUP_DIR, ret, strerror(ret));
        return NULL;
    }

    return names;
}

_PUBLIC_ int
authselect_backup_remove(const char *name)
{
    char *path;
    errno_t ret;

    INFO("Removing backup [%s]", name);

    path = format("%s/%s", AUTHSELECT_BACKUP_DIR, name);
    if (path == NULL) {
        return ENOMEM;
    }

    ret = dir_remove(path);
    if (ret != EOK) {
        ERROR("Unable to delete directory [%s] [%d]: %s",
              path, ret, strerror(ret));
    }

    return ret;
}

static errno_t
authselect_restore_system_configuration(const char *path)
{
    struct selinux_safe_copy table[] = {
        {FILE_CONFIG,      PATH_CONFIG_FILE},
        {FILE_SYSTEM,      PATH_SYMLINK_SYSTEM},
        {FILE_PASSWORD,    PATH_SYMLINK_PASSWORD},
        {FILE_FINGERPRINT, PATH_SYMLINK_FINGERPRINT},
        {FILE_SMARTCARD,   PATH_SYMLINK_SMARTCARD},
        {FILE_POSTLOGIN,   PATH_SYMLINK_POSTLOGIN},
        {FILE_NSSWITCH,    PATH_SYMLINK_NSSWITCH},
        {FILE_DCONF_DB,    PATH_SYMLINK_DCONF_DB},
        {FILE_DCONF_LOCK,  PATH_SYMLINK_DCONF_LOCK},
        {NULL, NULL},
    };
    errno_t ret;
    int i;

    for (i = 0; table[i].source != NULL; i++) {
        table[i].source = format("%s/%s", path, table[i].source);
        if (table[i].source == NULL) {
            ret = ENOMEM;
            goto done;
        }
    }

    ret = selinux_copy_files_safely(table, AUTHSELECT_DIR_MODE);

done:
    /* In case of an error in formatting the source file, it will be NULL
     * and the iteration will stop before we free the static data. */
    for (i = 0; table[i].source != NULL; i++) {
        free((char*)table[i].source);
    }

    return ret;
}

static errno_t
authselect_restore_authselect_configuration(const char *path)
{
    struct selinux_safe_copy table[] = {
        {FILE_CONFIG,      PATH_CONFIG_FILE},
        {FILE_SYSTEM,      PATH_SYSTEM},
        {FILE_PASSWORD,    PATH_PASSWORD},
        {FILE_FINGERPRINT, PATH_FINGERPRINT},
        {FILE_SMARTCARD,   PATH_SMARTCARD},
        {FILE_POSTLOGIN,   PATH_POSTLOGIN},
        {FILE_NSSWITCH,    PATH_NSSWITCH},
        {FILE_DCONF_DB,    PATH_DCONF_DB},
        {FILE_DCONF_LOCK,  PATH_DCONF_LOCK},
        {NULL, NULL},
    };
    errno_t ret;
    int i;

    for (i = 0; table[i].source != NULL; i++) {
        table[i].source = format("%s/%s", path, table[i].source);
        if (table[i].source == NULL) {
            ret = ENOMEM;
            goto done;
        }
    }

    ret = selinux_copy_files_safely(table, AUTHSELECT_DIR_MODE);
    if (ret != EOK) {
        ERROR("Unable to copy files [%d]: %s", ret, strerror(ret));
        goto done;
    }

    ret = authselect_symlinks_write();
    if (ret != EOK) {
        ERROR("Unable to create symbolic links [%d]: %s", ret, strerror(ret));
        goto done;
    }

    ret = authselect_profile_dconf_update();
    if (ret == ENOENT) {
        INFO("Dconf is not installed on your system");
    } else if (ret != EOK) {
        ERROR("Unable to update dconf database [%d]: %s", ret, strerror(ret));
        goto done;
    }

    ret = EOK;

done:
    /* In case of an error in formatting the source file, it will be NULL
     * and the iteration will stop before we free the static data. */
    for (i = 0; table[i].source != NULL; i++) {
        free((char*)table[i].source);
    }

    return ret;
}

_PUBLIC_ int
authselect_backup_restore(const char *name)
{
    char *confpath = NULL;
    char *path = NULL;
    errno_t ret;

    INFO("Restoring configuration from backup [%s]", name);

    path = format("%s/%s", AUTHSELECT_BACKUP_DIR, name);
    if (path == NULL) {
        ret = ENOMEM;
        goto done;
    }

    confpath = format("%s/%s", path, FILE_CONFIG);
    if (confpath == NULL) {
        ret = ENOMEM;
        goto done;
    }

    ret = file_exists(confpath);
    if (ret == EOK) {
        INFO("Backup [%s] contains authselect configuration", name);
        ret = authselect_restore_authselect_configuration(path);
    } else if (ret == ENOENT) {
        INFO("Backup [%s] contains non-authselect configuration", name);
        ret = authselect_restore_system_configuration(path);
    }

done:
    if (ret != EOK) {
        ERROR("Unable to restore [%s] [%d]: %s", name, ret, strerror(ret));
    }

    free(confpath);
    free(path);

    return ret;
}
