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

#ifndef _AUTHSELECT_PRIVATE_H_
#define _AUTHSELECT_PRIVATE_H_

#include <dirent.h>

#include "common/common.h"

/* Functions marked with this macro are exported to the library consumer. */
#define _PUBLIC_

#define AUTHSELECT_FILE_MODE       0644
#define AUTHSELECT_FILE_SIZE_LIMIT 4096
#define AUTHSELECT_CUSTOM_PREFIX   "custom/"

struct authselect_files {
    char *systemauth;
    char *passwordauth;
    char *smartcardauth;
    char *fingerprintauth;
    char *postlogin;
    char *nsswitch;
    char *dconfdb;
    char *dconflock;
};

struct authselect_profile {
    char *id;
    char *path;

    char *name;
    char *description;

    struct authselect_files files;
};

/**
 * Free authselect_files structure content but not the structure itself.
 */
void
authselect_files_free_content(struct authselect_files *files);

#endif /* _AUTHSELECT_PRIVATE_H_ */
