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

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>

#include "../common/gettext.h"
#include "../common/common.h"
#include "authselect.h"
#include "cli_tool.h"

static size_t
list_max_length(char **list)
{
    size_t max = 0;
    size_t len;
    int i;

    for (i = 0; list[i] != NULL; i++) {
        len = strlen(list[i]);
        if (max < len) {
            max = len;
        }
    }

    return max;
}

static errno_t
parse_profile_options(struct cli_cmdline *cmdline,
                      struct poptOption *options,
                      const char **_profile_id,
                      const char ***_optional)
{
    const char *profile_id;
    const char **optional;
    errno_t ret;
    int i;

    ret = cli_tool_popt_ex(cmdline, options, CLI_TOOL_OPT_OPTIONAL,
                           NULL, NULL, "PROFILE-ID", _("Profile identifier."),
                           &profile_id, true, NULL);
    if (ret != EOK) {
        ERROR("Unable to parse command arguments");
        return ret;
    }

    optional = malloc_zero_array(const char *, cmdline->argc);
    if (optional == NULL) {
        return ENOMEM;
    }

    for (i = 1; i < cmdline->argc; i++) {
        /* Skip options. */
        if (cmdline->argv[i][0] == '-') {
            continue;
        }

        optional[i - 1] = cmdline->argv[i];
    }

    *_profile_id = profile_id;
    *_optional = optional;

    return EOK;
}

static errno_t activate(struct cli_cmdline *cmdline)
{
    const char *profile_id;
    const char **optional;
    int enforce = 0;
    errno_t ret;

    struct poptOption options[] = {
        {"force", 'f', POPT_ARG_VAL, &enforce, 1, _("Enforce changes"), NULL },
        POPT_TABLEEND
    };

    ret = parse_profile_options(cmdline, options, &profile_id, &optional);
    if (ret != EOK) {
        return ret;
    }

    ret = authselect_activate(profile_id, optional, enforce);
    free(optional);
    if (ret == AUTHSELECT_ERR_FORCE_REQUIRED) {
        fprintf(stderr, _("\nSome unexpected changes to the configuration were "
                "detected.\nUse --force parameter if you want to overwrite "
                "these changes.\n"));
        return EINVAL;
    } else if (ret != EOK) {
        fprintf(stderr, _("Unable to activate profile [%d]: %s\n"),
                ret, strerror(ret));
        return ret;
    }

    return EOK;
}

static errno_t current(struct cli_cmdline *cmdline)
{
    char *profile_id;
    char **optional;
    errno_t ret;
    int i;

    ret = authselect_current(&profile_id, &optional);
    if (ret == ENOENT) {
        printf("No existing configuration detected.\n");
        return EOK;
    } else if (ret != EOK) {
        ERROR("Unable to get current configuration [%d]: %s",
              ret, strerror(ret));
        return ret;
    }

    printf("Profile ID: %s\n", profile_id);
    printf("Enabled optional modules:");

    if (optional == NULL || optional[0] == NULL) {
        printf(" None\n");
    } else {
        printf("\n");
    }

    for (i = 0; optional[i] != NULL; i++) {
        printf("--%s\n", optional[i]);
    }

    free(profile_id);
    authselect_optional_free(optional);

    return ret;
}

static errno_t check(struct cli_cmdline *cmdline)
{
    bool is_valid;
    errno_t ret;

    ret = authselect_check_conf(&is_valid);
    if (ret != EOK && ret != ENOENT) {
        ERROR("Unable to test current configuration [%d]: %s",
              ret, strerror(ret));

        return ret;
    }

    if (is_valid) {
        puts(_("Current configuration is valid."));
    } else {
        puts(_("Current configuration is not valid. "
               "It was probably modified outside authselect."));
    }

    return EOK;
}

static errno_t list(struct cli_cmdline *cmdline)
{
    struct authselect_profile *profile;
    char **profiles;
    errno_t ret;
    int maxlen;
    int i;

    profiles = authselect_list();
    if (profiles == NULL) {
        ERROR("Unable to get profile list!");
        return ENOMEM;
    }

    maxlen = list_max_length(profiles);

    for (i = 0; profiles[i] != NULL; i++) {
        profile = authselect_profile(profiles[i]);
        if (profile == NULL) {
            ERROR("Unable to get profile information!");
            ret = ENOMEM;
            goto done;
        }

        printf("- %-*s\t %s\n", maxlen, profiles[i],
               authselect_profile_name(profile));

        authselect_profile_free(profile);
    }

    ret = EOK;

done:
    authselect_list_free(profiles);
    return ret;
}

static errno_t show(struct cli_cmdline *cmdline)
{
    struct authselect_profile *profile;
    const char *profile_id;
    errno_t ret;

    ret = cli_tool_popt_ex(cmdline, NULL, CLI_TOOL_OPT_OPTIONAL,
                           NULL, NULL, "PROFILE-ID", _("Profile identifier."),
                           &profile_id, false, NULL);
    if (ret != EOK) {
        ERROR("Unable to parse command arguments");
        return ret;
    }

    profile = authselect_profile(profile_id);
    if (profile == NULL) {
        ERROR("Unable to get profile information!");
        return ENOMEM;
    }

    puts(authselect_profile_description(profile));

    authselect_profile_free(profile);

    return EOK;
}

static errno_t test(struct cli_cmdline *cmdline)
{
    struct authselect_files *files;
    const char *profile_id;
    const char **optional;
    const char *content;
    errno_t ret;

    ret = parse_profile_options(cmdline, NULL, &profile_id, &optional);
    if (ret != EOK) {
        return ret;
    }

    files = authselect_cat(profile_id, optional);
    if (files == NULL) {
        ERROR("Unable to get generated content!");
        return ENOMEM;
    }

    content = authselect_files_nsswitch(files);
    if (content == NULL) {
        printf("- nsswitch.conf: None\n\n");
    } else {
        printf("- nsswitch.conf:\n%s\n\n", content);
    }

    content = authselect_files_systemauth(files);
    if (content == NULL) {
        printf("- system-auth: None\n\n");
    } else {
        printf("- system-auth:\n%s\n\n", content);
    }

    content = authselect_files_passwordauth(files);
    if (content == NULL) {
        printf("- password-auth: None\n\n");
    } else {
        printf("- password-auth:\n%s\n\n", content);
    }

    content = authselect_files_smartcardauth(files);
    if (content == NULL) {
        printf("- smartcard-auth: None\n\n");
    } else {
        printf("- smartcard-auth:\n%s\n\n", content);
    }

    content = authselect_files_fingerprintauth(files);
    if (content == NULL) {
        printf("- fingerprint-auth: None\n\n");
    } else {
        printf("- fingerprint-auth:\n%s\n\n", content);
    }

    return EOK;
}

static errno_t
setup_gettext()
{
    char *c;

    c = setlocale(LC_ALL, "");
    if (c == NULL) {
        ERROR("Unable to set locale!");
    }

    errno = 0;
    c = bindtextdomain(PACKAGE, LOCALEDIR);
    if (c == NULL) {
        return errno;
    }

    errno = 0;
    c = textdomain(PACKAGE);
    if (c == NULL) {
        return errno;
    }

    return EOK;
}

int main(int argc, const char **argv)
{
    uid_t uid;
    errno_t ret;
    struct cli_route_cmd commands[] = {
        CLI_TOOL_COMMAND("select", "Select profile", activate),
        CLI_TOOL_COMMAND("list", "List available profiles", list),
        CLI_TOOL_COMMAND("show", "Show profile information", show),
        CLI_TOOL_COMMAND("current", "Get identificator of currently selected profile", current),
        CLI_TOOL_COMMAND("check", "Check if the current configration is valid", check),
        CLI_TOOL_COMMAND("test", "Print changes that would be otherwise written", test),
        CLI_TOOL_LAST
    };

    ret = setup_gettext();
    if (ret != EOK) {
        fprintf(stderr, "Unable to setup gettext!\n");
        return EXIT_FAILURE;
    }

    uid = getuid();
    if (uid != 0) {
        fprintf(stderr, _("Authselect can only be run as root!\n"));
        return EXIT_FAILURE;
    }

    return cli_tool_main(argc, argv, commands, NULL);
}
