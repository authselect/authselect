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

#ifndef _UTIL_H_
#define _UTIL_H_

/**
 * Many of the utility functions are not as effective as they can be but
 * this is OK since authselect works only with small configuration files
 * therefore we can prefer clean and simple code over performance.
 */

#include "common/common.h"
#include "lib/util/file.h"
#include "lib/util/string.h"
#include "lib/util/string_array.h"
#include "lib/util/template.h"
#include "lib/util/textfile.h"

#endif /* _UTIL_H_ */
