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

#ifndef _COMMON_H_
#define _COMMON_H_

#include "config.h"

#include <stdarg.h>

#include "gettext.h"
#include "authselect.h"

#define _(s) gettext(s)

/* Define error type and success errno value. */

#ifndef HAVE_ERRNO_T
#define HAVE_ERRNO_T
typedef int errno_t;
#endif

#ifndef EOK
#define EOK 0
#endif

/* Debugging facility. */

void set_debug_fn(authselect_debug_fn fn, void *pvt);

void debug(enum authselect_debug level,
           const char *file,
           unsigned long line,
           const char *function,
           const char *fmt,
           ...);

#define INFO(fmt, ...)                                                        \
    debug(AUTHSELECT_INFO, __FILE__, __LINE__, __FUNCTION__,                  \
          gettext(fmt), ## __VA_ARGS__)

#define WARN(fmt, ...)                                                        \
    debug(AUTHSELECT_WARNING, __FILE__, __LINE__, __FUNCTION__,               \
          gettext(fmt), ## __VA_ARGS__)

#define ERROR(fmt, ...)                                                       \
    debug(AUTHSELECT_ERROR, __FILE__, __LINE__, __FUNCTION__,                 \
          gettext(fmt), ## __VA_ARGS__)

/* Wrapper around aprintf to simplify error handling. */
char *format(const char *fmt, ...);
char *vaformat(const char *fmt, va_list va);

#endif /* _COMMON_H_ */
