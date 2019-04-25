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

#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

#include <sys/stat.h>

#include "common/errno_t.h"

/**
 * Generate output from a template.
 *
 * @param template    Template.
 * @param features    Features to enable.
 *
 * @return Generated content or NULL on error.
 */
char *
template_generate(const char *template,
                  const char **features);

/**
 * Find all features available within the @template and return them in
 * NULL-terminated array.
 *
 * @param template    Template.
 *
 * @return List of features in NULL-terminated array or NULL on error.
 */
char **
template_list_features(const char *template);

/**
 * Write generated file preamble together with its content to a file.
 * If the file does not exist, it is created, otherwise its content
 * is truncated. The file mode is set to @mode.
 *
 * @param filepath     Path to the file.
 * @param content      Content to write.
 * @param mode         Mode to create the file with.
 * @param timestamp    Time when the content was generated that
 *                     will be written to preambule.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
template_write(const char *filepath,
               const char *content,
               mode_t mode,
               time_t timestamp);

/**
 * Write generated file preamble together with its content to a temporary file.
 * The temporary file name is returned in @_tmpfile.
 * The file mode is set to @mode.
 *
 * @param filepath     Path to the file.
 * @param content      Content to write.
 * @param mode         Mode to create the file with.
 * @param timestamp    Time when the content was generated that
 *                     will be written to preambule.
 * @param _tmpfile     Name of created temporary file.
 *
 * @return EOK on success, other errno code on error.
 */
errno_t
template_write_temporary(const char *filepath,
                         const char *content,
                         mode_t mode,
                         time_t timestamp,
                         char **_tmpfile);

/**
 * Validate previously generated and written file content.
 *
 * @return True if the content was not modified, false otherwise.
 */
bool
template_validate_written_content(const char *file_content,
                                  const char *expected);

#endif /* _TEMPLATE_H_ */
