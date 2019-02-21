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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <selinux/selinux.h>
#include <selinux/label.h>

#include "common/common.h"
#include "lib/util/file.h"

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
