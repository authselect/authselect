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

#include <stdlib.h>

#include "lib/authselect_private.h"
#include "lib/files/files.h"

_PUBLIC_ const char *
authselect_files_nsswitch(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->nsswitch;
}

_PUBLIC_ const char *
authselect_files_systemauth(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->systemauth;
}

_PUBLIC_ const char *
authselect_files_passwordauth(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->passwordauth;
}

_PUBLIC_ const char *
authselect_files_smartcardauth(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->smartcardauth;
}

_PUBLIC_ const char *
authselect_files_fingerprintauth(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->fingerprintauth;
}

_PUBLIC_ const char *
authselect_files_postlogin(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->postlogin;
}

_PUBLIC_ const char *
authselect_files_dconf_db(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->dconfdb;
}

_PUBLIC_ const char *
authselect_files_dconf_lock(const struct authselect_files *files)
{
    if (files == NULL) {
        return NULL;
    }

    return files->dconflock;
}

_PUBLIC_ void
authselect_files_free(struct authselect_files *files)
{
    if (files == NULL) {
        return;
    }

    if (files->systemauth != NULL) {
        free(files->systemauth);
    }

    if (files->passwordauth != NULL) {
        free(files->passwordauth);
    }

    if (files->smartcardauth != NULL) {
        free(files->smartcardauth);
    }

    if (files->fingerprintauth != NULL) {
        free(files->fingerprintauth);
    }

    if (files->postlogin != NULL) {
        free(files->postlogin);
    }

    if (files->nsswitch != NULL) {
        free(files->nsswitch);
    }

    if (files->dconfdb != NULL) {
        free(files->dconfdb);
    }

    if (files->dconflock != NULL) {
        free(files->dconflock);
    }

    free(files);
}
