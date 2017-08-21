/*
    Authors:
        Pavel Březina <pbrezina@redhat.com>

    Copyright (C) 2017 Red Hat

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
#include <sys/stat.h>
#include <unistd.h>

#include "authselect.h"
#include "lib/authselect_private.h"
#include "lib/authselect_files.h"
#include "lib/authselect_util.h"

static errno_t
write_generated_files(struct authselect_profile *profile,
                      const char **optional)
{
    struct authselect_files *files;
    errno_t ret;
    int i;

    ret = authselect_files_generate(profile, optional, &files);
    if (ret != EOK) {
        ERROR("Unable to generate content [%d]: %s", ret, strerror(ret));
        return ret;
    }

    struct authselect_generated generated[] = GENERATED_FILES(files);

    for (i = 0; generated[i].path != NULL; i++) {
        ret = create_textfile(generated[i].path, generated[i].content);
        if (ret != EOK) {
            goto done;
        }
    }

done:
    authselect_files_free(files);

    return ret;
}

static errno_t
write_symbolic_links()
{
    struct authselect_symlink symlinks[] = SYMLINK_FILES;
    mode_t oldmask;
    errno_t ret;
    int i;

    oldmask = umask(0644);

    for (i = 0; symlinks[i].name != NULL; i++) {
        INFO("Creating symbolic link [%s] to [%s]",
             symlinks[i].name, symlinks[i].dest);

        ret = unlink(symlinks[i].name);
        if (ret != 0 && errno != ENOENT) {
            ret = errno;
            ERROR("Unable to overwrite file [%s] [%d]: %s",
                  symlinks[i].name, ret, strerror(ret));
            goto done;
        }

        ret = symlink(symlinks[i].dest, symlinks[i].name);
        if (ret != 0) {
            ret = errno;
            ERROR("Unable to create symbolic link [%s] [%d]: %s",
                  symlinks[i].name, ret, strerror(ret));
            goto done;
        }
    }

    ret = EOK;

done:
    umask(oldmask);

    return ret;
}

static errno_t
write_config(const char *profile_id,
             const char **optional)
{
    char *buf;
    errno_t ret;
    int i;

    buf = buffer_append_line(NULL, profile_id);
    if (buf == NULL) {
        return ENOMEM;
    }

    for (i = 0; optional[i] != NULL; i++) {
        buf = buffer_append_line(buf, optional[i]);
        if (buf == NULL) {
            return ENOMEM;
        }
    }

    ret = create_textfile(PATH_CONFIG_FILE, buf);
    free(buf);

    return ret;
}

static errno_t
authselect_activate_profile(struct authselect_profile *profile,
                            const char **optional)
{
    errno_t ret;

    ret = write_generated_files(profile, optional);
    if (ret != EOK) {
        ERROR("Unable to write generated files [%d]: %s", ret, strerror(ret));
        goto done;
    }

    ret = write_config(profile->id, optional);
    if (ret != EOK) {
        ERROR("Unable to write configuration [%d]: %s", ret, strerror(ret));
        goto done;
    }

    ret = write_symbolic_links();
    if (ret != EOK) {
        ERROR("Unable to create symbolic links [%d]: %s", ret, strerror(ret));
        goto done;
    }

done:
    return ret;
}

_PUBLIC_ int
authselect_activate(const char *profile_id,
                    const char **optional,
                    bool force_override)
{
    struct authselect_profile *profile;
    bool symlink_exist;
    bool is_valid;
    errno_t ret;

    INFO("Trying to activate profile [%s]", profile_id);

    ret = authselect_profile(profile_id, &profile);
    if (ret != EOK) {
        ERROR("Unable to find profile [%s] [%d]: %s",
              profile_id, ret, strerror(ret));
        return ret;
    }

    if (force_override) {
        INFO("Enforcing activation!");
        ret = authselect_activate_profile(profile, optional);
        goto done;
    }

    /* First, check that current configuration is valid. */
    ret = authselect_check_conf(&is_valid);
    if (ret != EOK && ret != ENOENT) {
        ERROR("Unable to check configuration [%d]: %s", ret, strerror(ret));
        goto done;
    }

    if (!is_valid) {
        ERROR("Unexpected changes to the configuration were detected.");
        ERROR("Refusing to activate profile unless those changes are removed "
              "or overwrite is requested.");
        ret = AUTHSELECT_ERR_FORCE_REQUIRED;
        goto done;
    }

    /* If no configuration is present, check for existing files. */
    if (ret == ENOENT) {
        ret = authselect_check_symlinks_presence(&symlink_exist);
        if (ret != EOK) {
            ERROR("Unable to check for presence of symbolic links [%d]: %s",
                  ret, strerror(ret));
            goto done;
        }

        if (symlink_exist) {
            ERROR("File that needs to be overwritten was found");
            ERROR("Refusing to activate profile unless this file is removed "
                  "or overwrite is requested.");
            ret = AUTHSELECT_ERR_FORCE_REQUIRED;
            goto done;
        }
    }

    ret = authselect_activate_profile(profile, optional);

done:
    if (ret != EOK && ret != AUTHSELECT_ERR_FORCE_REQUIRED) {
        ERROR("Unable to activate profile [%s] [%d]: %s",
              profile_id, ret, strerror(ret));
    }

    authselect_profile_free(profile);

    return ret;
}
