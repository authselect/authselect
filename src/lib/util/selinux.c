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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <selinux/selinux.h>
#include <selinux/label.h>
#include <sys/stat.h>

#include "common/common.h"
#include "lib/util/file.h"
#include "lib/util/selinux.h"
#include "lib/util/string_array.h"

errno_t
selinux_get_default_context(const char *path,
                            char **_context)
{
    struct selabel_handle *handle;
    char *context;
    int ret;

    handle = selabel_open(SELABEL_CTX_FILE, NULL, 0);
    if (handle == NULL) {
        ret = errno;
        ERROR("Unable to create selabel context [%d]: %s", ret, strerror(ret));
        return ret;
    }

    ret = selabel_lookup(handle, &context, path, 0);
    if (ret < 0 && errno == ENOENT) {
        context = NULL;
    } else if (ret != 0) {
        ret = errno;
        ERROR("Unable to lookup selinux context [%d]: %s", ret, strerror(ret));
        goto done;
    }

    INFO("Found default selinux context for [%s]: %s",
         path, context == NULL ? "NULL" : context);

    *_context = context;

    ret = EOK;

done:
    selabel_close(handle);

    return ret;
}

errno_t
selinux_get_context(const char *path,
                    char **_context)
{
    char *context;
    int ret;

    ret = getfilecon(path, &context);
    if (ret < 0 && errno == ENOENT) {
        return selinux_get_default_context(path, _context);
    } else if (ret < 0) {
        ret = errno;
        ERROR("Unable to obtain selinux context for [%s] [%d]: %s",
              path, ret, strerror(ret));

        return ret;
    }

    INFO("Found selinux context for [%s]: %s",
         path, context == NULL ? _("not set") : context);

    *_context = context;

    return EOK;
}

errno_t
selinux_mkstemp_for(const char *filepath,
                    mode_t mode,
                    char **_tmpfile)
{
    char *original_context = NULL;
    char *default_context = NULL;
    char *tmpfile;
    errno_t ret;
    int seret;

    if (is_selinux_enabled() != 1) {
        return file_mktmp_for(filepath, mode, _tmpfile);
    }

    seret = getfscreatecon(&original_context);
    if (seret != 0) {
        ERROR("Unable to get current fscreate selinux context!");
        return EIO;
    }

    ret = selinux_get_context(filepath, &default_context);
    if (ret != EOK) {
        ERROR("Unable to get default selinux context for [%s] [%d]: %s!",
              filepath, ret, strerror(ret));
        goto done;
    }

    /* Set desired fs create context. */
    seret = setfscreatecon(default_context);
    if (seret != 0) {
        ERROR("Unable to set fscreate selinux context!");
        ret = EIO;
        goto done;
    }

    ret = file_mktmp_for(filepath, mode, &tmpfile);

    /* Restore original fs create context. */
    seret = setfscreatecon(original_context);
    if (seret != 0) {
        ERROR("Unable to restore fscreate selinux context!");
        ret = EIO;
        goto done;
    }

    /* Check result of file_mktmp_for() */
    if (ret != EOK) {
        free(tmpfile);
        goto done;
    }

    *_tmpfile = tmpfile;

    ret = EOK;

done:
    if (original_context != NULL) {
        freecon(original_context);
    }

    if (default_context != NULL) {
        freecon(default_context);
    }

    return ret;
}

errno_t
selinux_file_copy(const char *source,
                  const char *destdir,
                  const char *destname,
                  mode_t dir_mode)
{
    char *original_context = NULL;
    char *default_context = NULL;
    errno_t ret;
    int seret;

    if (is_selinux_enabled() != 1) {
        return file_copy(source, destdir, destname, dir_mode);
    }

    seret = getfscreatecon(&original_context);
    if (seret != 0) {
        ERROR("Unable to get current fscreate selinux context!");
        return EIO;
    }

    ret = selinux_get_context(source, &default_context);
    if (ret != EOK) {
        ERROR("Unable to get default selinux context for [%s] [%d]: %s!",
              source, ret, strerror(ret));
        goto done;
    }

    /* Set desired fs create context. */
    seret = setfscreatecon(default_context);
    if (seret != 0) {
        ERROR("Unable to set fscreate selinux context!");
        ret = EIO;
        goto done;
    }

    ret = file_copy(source, destdir, destname, dir_mode);

    /* Restore original fs create context. */
    seret = setfscreatecon(original_context);
    if (seret != 0) {
        ERROR("Unable to restore fscreate selinux context!");
        ret = EIO;
        goto done;
    }

    /* Check result of textfile_copy() */
    if (ret != EOK) {
        goto done;
    }

    ret = EOK;

done:
    if (original_context != NULL) {
        freecon(original_context);
    }

    if (default_context != NULL) {
        freecon(default_context);
    }

    return ret;
}

errno_t
selinux_mkstemp_copy(const char *source,
                     const char *destdir,
                     const char *destname,
                     mode_t dir_mode,
                     char **_tmpfile)
{
    char *original_context = NULL;
    char *default_context = NULL;
    errno_t ret;
    int seret;

    if (is_selinux_enabled() != 1) {
        return file_mktmp_copy(source, destdir, destname, dir_mode, _tmpfile);
    }

    seret = getfscreatecon(&original_context);
    if (seret != 0) {
        ERROR("Unable to get current fscreate selinux context!");
        return EIO;
    }

    ret = selinux_get_context(source, &default_context);
    if (ret != EOK) {
        ERROR("Unable to get default selinux context for [%s] [%d]: %s!",
              source, ret, strerror(ret));
        goto done;
    }

    /* Set desired fs create context. */
    seret = setfscreatecon(default_context);
    if (seret != 0) {
        ERROR("Unable to set fscreate selinux context!");
        ret = EIO;
        goto done;
    }

    ret = file_mktmp_copy(source, destdir, destname, dir_mode, _tmpfile);

    /* Restore original fs create context. */
    seret = setfscreatecon(original_context);
    if (seret != 0) {
        ERROR("Unable to restore fscreate selinux context!");
        ret = EIO;
        goto done;
    }

    /* Check result of textfile_copy() */
    if (ret != EOK) {
        goto done;
    }

    ret = EOK;

done:
    if (original_context != NULL) {
        freecon(original_context);
    }

    if (default_context != NULL) {
        freecon(default_context);
    }

    return ret;
}

static errno_t
parse_destination_path(const char *destination,
                       char **_dir,
                       char **_name)
{
    const char *cname;
    char *name;
    char *dir;

    dir = file_get_parent_directory(destination);
    if (dir == NULL) {
        return ENOMEM;
    }

    cname = file_get_basename(destination);
    if (cname == NULL) {
        free(dir);
        return EINVAL;
    }

    name = strdup(cname);
    if (name == NULL) {
        free(dir);
        return ENOMEM;
    }

    *_dir = dir;
    *_name = name;

    return EOK;
}

errno_t
selinux_copy_files_safely(struct selinux_safe_copy *table,
                          mode_t dir_mode)
{
    char **tmpfiles = NULL;
    char **names = NULL;
    char **dirs = NULL;
    errno_t ret;
    int i;

    for (i = 0; table[i].source != NULL; i++) {
        /* Just counting. */
    }

    tmpfiles = string_array_create(i);
    if (tmpfiles == NULL) {
        ret = ENOMEM;
        goto done;
    }

    dirs = string_array_create(i);
    if (dirs == NULL) {
        ret = ENOMEM;
        goto done;
    }

    names = string_array_create(i);
    if (names == NULL) {
        ret = ENOMEM;
        goto done;
    }

    /* Parse destination. */
    for (i = 0; table[i].source != NULL; i++) {
        ret = parse_destination_path(table[i].destination, &dirs[i], &names[i]);
        if (ret != EOK) {
            goto done;
        }
    }

    /* First, write content into temporary files, so we can safely fail
     * on error without overwriting destination files. */
    for (i = 0; table[i].source != NULL; i++) {
        INFO("Writing temporary file for [%s]", table[i].destination);
        ret = selinux_mkstemp_copy(table[i].source, dirs[i], names[i],
                                   dir_mode, &tmpfiles[i]);
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
    for (i = 0; table[i].source != NULL; i++) {
        INFO("Renaming [%s] to [%s]", tmpfiles[i], table[i].destination);
        ret = rename(tmpfiles[i], table[i].destination);
        if (ret != 0) {
            ret = errno;
            ERROR("Unable to rename [%s] to [%s] [%d]: %s",
                  tmpfiles[i], table[i].destination, ret, strerror(ret));
            goto done;
        }
    }

    ret = EOK;

done:
    if (ret != EOK && tmpfiles != NULL) {
        for (i = 0; table[i].source != NULL; i++) {
            if (tmpfiles[i] != NULL) {
                unlink(tmpfiles[i]);
            }
        }
    }

    string_array_free(tmpfiles);
    string_array_free(names);
    string_array_free(dirs);

    return ret;
}
