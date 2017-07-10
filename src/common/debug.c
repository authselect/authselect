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

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

#include "authselect.h"

authselect_debug_fn debug_fn;
void *debug_fn_pvt;

void set_debug_fn(authselect_debug_fn fn, void *pvt)
{
    debug_fn = fn;
    debug_fn_pvt = pvt;
}

void debug(enum authselect_debug level,
           const char *function,
           const char *fmt,
           ...)
{
    char msg[255];
    va_list va;
    int ret;

    if (debug_fn == NULL) {
        return;
    }

    va_start(va, fmt);
    ret = vsnprintf(msg, sizeof(msg) / sizeof(char), fmt, va);
    va_end(va);

    if (ret >= sizeof(msg) / sizeof(char)) {
        debug_fn(debug_fn_pvt, AUTHSELECT_ERROR, function,
                 "Internal error: Message too long!");
        return;
    }

    if (ret < 0) {
        debug_fn(debug_fn_pvt, AUTHSELECT_ERROR, function,
                 "Unable to construct message!");
        return;
    }

    debug_fn(debug_fn_pvt, level, function, msg);
}
