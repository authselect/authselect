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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "authselect.h"
#include "common/common.h"
#include "lib/constants.h"
#include "lib/util/util.h"
#include "lib/profiles/profiles.h"

_PUBLIC_ int
authselect_profile(const char *profile_id,
                   struct authselect_profile **_profile)
{
    return authselect_profile_read(profile_id, AUTHSELECT_PROFILE_ANY,
                                   _profile);
}

_PUBLIC_ const char *
authselect_profile_id(const struct authselect_profile *profile)
{
    if (profile == NULL) {
        return NULL;
    }

    return profile->id;
}

_PUBLIC_ const char *
authselect_profile_name(const struct authselect_profile *profile)
{
    if (profile == NULL) {
        return NULL;
    }

    return profile->name;
}

_PUBLIC_ const char *
authselect_profile_path(const struct authselect_profile *profile)
{
    if (profile == NULL) {
        return NULL;
    }

    return profile->path;
}

_PUBLIC_ const char *
authselect_profile_description(const struct authselect_profile *profile)
{
    if (profile == NULL) {
        return NULL;
    }

    return profile->description;
}

_PUBLIC_ char *
authselect_profile_requirements(const struct authselect_profile *profile,
                                const char **features)
{
    if (profile == NULL) {
        return NULL;
    }

    return template_generate(profile->requirements, features);
}

_PUBLIC_ char **
authselect_profile_nsswitch_maps(const struct authselect_profile *profile,
                                 const char **features)
{
    char *template;
    char **maps;
    errno_t ret;

    if (profile == NULL) {
        return NULL;
    }

    template = template_generate(profile->files->nsswitch, features);
    if (template == NULL) {
        ERROR("Unable to generate nsswitch.conf");
        return NULL;
    }

    ret = authselect_system_nsswitch_find_maps(template, &maps);
    free(template);
    if (ret != EOK) {
        ERROR("Unable to find nsswitch maps [%d]: %s", ret, strerror(ret));
        return NULL;
    }

    return maps;
}

_PUBLIC_ void
authselect_profile_free(struct authselect_profile *profile)
{
    if (profile == NULL) {
        return;
    }

    if (profile->id != NULL) {
        free(profile->id);
    }

    if (profile->path != NULL) {
        free(profile->path);
    }

    if (profile->name != NULL) {
        free(profile->name);
    }

    if (profile->description != NULL) {
        free(profile->description);
    }

    if (profile->requirements != NULL) {
        free(profile->requirements);
    }

    authselect_files_free(profile->files);

    memset(profile, 0, sizeof(struct authselect_profile));

    free(profile);

    return;
}

static char **
authselect_profile_files_get(const char *profile_path)
{
    const char *files[] = {FILES_ALL, NULL};
    char **paths;
    char *path;
    errno_t ret;
    int i;

    paths = string_array_create(sizeof(files) / sizeof(const char *));
    if (paths == NULL) {
        return NULL;
    }

    for (i = 0; files[i] != NULL; i++) {
        path = format("%s/%s", profile_path, files[i]);
        if (path == NULL) {
            ret = ENOMEM;
            goto done;
        }

        paths = string_array_add_value(paths, path, true);
        free(path);
        if (paths == NULL) {
            ret = ENOMEM;
            goto done;
        }
    }

    ret = EOK;

done:
    if (ret != EOK) {
        string_array_free(paths);
        return NULL;
    }

    return paths;
}

static errno_t
authselect_profile_symlinks_add(const char *profile_path,
                               char ***symlinks,
                               const char **from)
{
    char *target;
    int i;

    if (from == NULL) {
        return EOK;
    }

    for (i = 0; from[i] != NULL; i++) {
        target = format("%s/%s", profile_path, from[i]);
        if (target == NULL) {
            return ENOMEM;
        }

        *symlinks = string_array_add_value(*symlinks, target, true);
        free(target);
        if (*symlinks == NULL) {
            return ENOMEM;
        }
    }

    return EOK;
}

static char **
authselect_profile_symlinks_get(const char *profile_path,
                                uint32_t flags,
                                const char **symlinks)
{
    const char *sym_meta[]     = {FILES_META, NULL};
    const char *sym_nsswitch[] = {FILES_NSSWITCH, NULL};
    const char *sym_pam[]      = {FILES_PAM, NULL};
    const char *sym_dconf[]    = {FILES_DCONF, NULL};
    char **targets;
    errno_t ret;

    targets = string_array_create(10);
    if (targets == NULL) {
        ret = ENOMEM;
        goto done;
    }

    if (flags & AUTHSELECT_SYMLINK_META) {
        ret = authselect_profile_symlinks_add(profile_path, &targets,
                                              sym_meta);
        if (ret != EOK) {
            goto done;
        }
    }

    if (flags & AUTHSELECT_SYMLINK_NSSWITCH) {
        ret = authselect_profile_symlinks_add(profile_path, &targets,
                                              sym_nsswitch);
        if (ret != EOK) {
            goto done;
        }
    }

    if (flags & AUTHSELECT_SYMLINK_PAM) {
        ret = authselect_profile_symlinks_add(profile_path, &targets,
                                              sym_pam);
        if (ret != EOK) {
            goto done;
        }
    }

    if (flags & AUTHSELECT_SYMLINK_DCONF) {
        ret = authselect_profile_symlinks_add(profile_path, &targets,
                                              sym_dconf);
        if (ret != EOK) {
            goto done;
        }
    }

    ret = authselect_profile_symlinks_add(profile_path, &targets,
                                          symlinks);
    if (ret != EOK) {
        goto done;
    }

done:
    if (ret != EOK) {
        string_array_free(targets);
        return NULL;
    }

    return targets;
}

static errno_t
authselect_profile_create_empty(const char *path, char **filepaths)
{
    errno_t ret;
    int i;

    INFO("Creating empty profile at [%s]", path);

    ret = file_make_path(path, AUTHSELECT_DIR_MODE);
    if (ret != EOK) {
        ERROR("Unable to make path [%s] [%d]: %s", path, ret, strerror(ret));
        return ret;
    }

    for (i = 0; filepaths[i] != NULL; i++) {
        ret = textfile_write(filepaths[i], "", AUTHSELECT_FILE_MODE);
        if (ret != EOK) {
            ERROR("Unable to write to [%s] [%d]: %s",
                  filepaths[i], ret, strerror(ret));
            return ret;
        }
    }

    return EOK;
}

static errno_t
authselect_profile_create_from_source(const char *filename,
                                      const char *destination,
                                      const char *content,
                                      char **symlinks)
{
    const char *symname;
    errno_t ret;
    int i;

    for (i = 0; symlinks[i] != NULL; i++) {
        symname = file_get_basename(symlinks[i]);
        if (symname == NULL) {
            ERROR("There is no filename in [%s]", symlinks[i]);
            return EINVAL;
        }

        if (strcmp(filename, symname) == 0) {
            /* Create symlink instead of copying the file. */
            ret = symlink(symlinks[i], destination);
            if (ret != 0) {
                ret = errno;
                ERROR("Unable to create symbolic link [%s] to [%s] [%d]: %s",
                      destination, symlinks[i], ret, strerror(ret));
                return ret;
            }

            return EOK;
        }
    }

    ret = textfile_write(destination, content, AUTHSELECT_FILE_MODE);
    if (ret != EOK) {
        ERROR("Unable to write to [%s] [%d]: %s",
              destination, ret, strerror(ret));
        return ret;
    }

    return EOK;
}

static errno_t
authselect_profile_create_from(const char *path,
                               char **filepaths,
                               const char *base_id,
                               enum authselect_profile_type base_type,
                               uint32_t symlink_flags,
                               const char **symlinks)
{
    struct authselect_profile *base;
    char **symlink_targets;
    const char *filename;
    errno_t ret;
    int i, j;

    INFO("Creating new profile from \"%s\" at [%s]", base_id, path);

    ret = authselect_profile_read(base_id, base_type, &base);
    if (ret != EOK) {
        ERROR("Unable to read base profile [%s] [%d]: %s",
              base_id, ret, strerror(ret));
        return ret;
    }

    symlink_targets = authselect_profile_symlinks_get(base->path, symlink_flags,
                                                      symlinks);
    if (symlink_targets == NULL) {
        ERROR("Unable to resolve symbolic links names");
        ret = ENOMEM;
        goto done;
    }

    ret = file_make_path(path, AUTHSELECT_DIR_MODE);
    if (ret != EOK) {
        ERROR("Unable to make path [%s] [%d]: %s", path, ret, strerror(ret));
        goto done;
    }

    struct authselect_generated profile_files[] = PROFILE_FILES(base->files);
    for (i = 0; filepaths[i] != NULL; i++) {
        filename = file_get_basename(filepaths[i]);
        if (filename == NULL) {
            ERROR("There is no filename in [%s]", filepaths[i]);
            ret = EINVAL;
            goto done;
        }

        if (strcmp(filename, FILE_README) == 0) {
            ret = authselect_profile_create_from_source(filename,
                        filepaths[i], base->description,
                        symlink_targets);
            if (ret != EOK) {
                ERROR("Unable to create [%s] [%d]: %s",
                      filepaths[i], ret, strerror(ret));
                goto done;
            }

            continue;
        }

        if (strcmp(filename, FILE_REQUIREMENT) == 0) {
            ret = authselect_profile_create_from_source(filename,
                        filepaths[i], base->requirements,
                        symlink_targets);
            if (ret != EOK) {
                ERROR("Unable to create [%s] [%d]: %s",
                      filepaths[i], ret, strerror(ret));
                goto done;
            }

            continue;
        }

        for (j = 0; profile_files[j].path != NULL; j++) {
            if (strcmp(filename, profile_files[j].path) == 0) {
                ret = authselect_profile_create_from_source(filename,
                            filepaths[i], profile_files[j].content,
                            symlink_targets);
                if (ret != EOK) {
                    ERROR("Unable to create [%s] [%d]: %s",
                          filepaths[i], ret, strerror(ret));
                    goto done;
                }

                break;
            }

            WARN("Unknown file name [%s]", filepaths[i]);
        }
    }

    ret = EOK;

done:
    authselect_profile_free(base);

    return ret;
}

_PUBLIC_ int
authselect_profile_create(const char *name,
                          enum authselect_profile_type type,
                          const char *base_id,
                          enum authselect_profile_type base_type,
                          uint32_t symlink_flags,
                          const char **symlinks,
                          char **_path)
{
    char **filepaths = NULL;
    char *path = NULL;
    errno_t ret;
    int i;

    if (string_is_empty(name)) {
        ERROR("Name can not be empty");
        return EINVAL;
    }

    switch (type) {
    case AUTHSELECT_PROFILE_VENDOR:
        path = format("%s/%s", DIR_VENDOR_PROFILES, name);
        break;
    case AUTHSELECT_PROFILE_CUSTOM:
        path = format("%s/%s", DIR_CUSTOM_PROFILES, name);
        break;
    case AUTHSELECT_PROFILE_DEFAULT:
        ERROR("Default profile can not be created");
        return EINVAL;
    case AUTHSELECT_PROFILE_ANY:
        ERROR("Value AUTHSELECT_PROFILE_ANY is invalid in this context");
        return EINVAL;
    }

    if (path == NULL) {
        ERROR("Unable to create profile path: out of memory");
        return ENOMEM;
    }

    ret = file_exists(path);
    if (ret == EOK) {
        ERROR("Profile \"%s\" already exist at [%s]", name, path);
        ret = EEXIST;
        goto done;
    } else if (ret != ENOENT) {
        ERROR("Unable to access [%s] [%d]: %s", path, ret, strerror(ret));
        goto done;
    }

    filepaths = authselect_profile_files_get(path);
    if (filepaths == NULL) {
        ERROR("Unable to create file name: out of memory");
        ret = ENOMEM;
        goto done;
    }

    if (base_id == NULL) {
        ret = authselect_profile_create_empty(path, filepaths);
        if (ret != EOK) {
            ERROR("Unable to create empty profile [%d]: %s",
                  ret, strerror(ret));
            goto done;
        }
    } else {
        ret = authselect_profile_create_from(path, filepaths, base_id,
                                             base_type, symlink_flags,
                                             symlinks);
        if (ret != EOK) {
            ERROR("Unable to create profile [%d]: %s",
                  ret, strerror(ret));
            goto done;
        }
    }

    if (_path != NULL) {
        *_path = path;
    } else {
        free(path);
    }

    ret = EOK;

done:
    if (ret != EOK) {
        for (i = 0; filepaths != NULL && filepaths[i] != NULL; i++) {
            unlink(filepaths[i]);
        }

        rmdir(path);
        free(path);
    }

    string_array_free(filepaths);

    return ret;
}
