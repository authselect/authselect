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

#ifndef _SELINUX_H_
#define _SELINUX_H_

#include "common/errno_t.h"

/**
 * Get default security context for @path.
 *
 * @param path Path to the file.
 *
 * @return EOK on success, ENOENT if context was not found, other errno code
 *         is returned on failure.
 */
errno_t
selinux_get_default_context(const char *path);

/**
 * Create temporary file created on @filepath.XXXXXX with security context
 * set to default security context of @filepath.
 *
 * @param filepath File for which a temporary file should be created.
 * @param _tmpfile Create temporary file.
 *
 * @return EOK on success, other errno code on failure.
 */
errno_t
selinux_mkstemp_of(const char *filepath,
                   char **_tmpfile);

#endif /* _SELINUX_H_ */
