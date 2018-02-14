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
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>

#include "authselect.h"
#include "common/common.h"
#include "cli/cli_tool.h"

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
                      const char ***_features)
{
    const char *profile_id;
    const char **features;
    errno_t ret;
    int i;

    ret = cli_tool_popt_ex(cmdline, options, CLI_TOOL_OPT_OPTIONAL,
                           NULL, NULL, "PROFILE-ID", _("Profile identifier."),
                           &profile_id, true, NULL);
    if (ret != EOK) {
        ERROR("Unable to parse command arguments");
        return ret;
    }

    features = malloc_zero_array(const char *, cmdline->argc);
    if (features == NULL) {
        return ENOMEM;
    }

    for (i = 1; i < cmdline->argc; i++) {
        /* Skip options. */
        if (cmdline->argv[i][0] == '-') {
            continue;
        }

        features[i - 1] = cmdline->argv[i];
    }

    *_profile_id = profile_id;
    *_features = features;

    return EOK;
}

static errno_t activate(struct cli_cmdline *cmdline)
{
    const char *profile_id;
    const char **features;
    int enforce = 0;
    errno_t ret;

    struct poptOption options[] = {
        {"force", 'f', POPT_ARG_VAL, &enforce, 1, _("Enforce changes"), NULL },
        POPT_TABLEEND
    };

    ret = parse_profile_options(cmdline, options, &profile_id, &features);
    if (ret != EOK) {
        return ret;
    }

    ret = authselect_activate(profile_id, features, enforce);
    free(features);
    if (ret == EEXIST) {
        fprintf(stderr, _("\nSome unexpected changes to the configuration were "
                "detected.\nUse --force parameter if you want to overwrite "
                "these changes.\n"));
        return ret;
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
    char **features;
    errno_t ret;
    int i;

    ret = cli_tool_popt(cmdline, NULL, CLI_TOOL_OPT_OPTIONAL, NULL, NULL);
    if (ret != EOK) {
        ERROR("Unable to parse command arguments");
        return ret;
    }

    ret = authselect_current_configuration(&profile_id, &features);
    if (ret == ENOENT) {
        printf(_("No existing configuration detected.\n"));
        return ret;
    } else if (ret != EOK) {
        ERROR("Unable to get current configuration [%d]: %s",
              ret, strerror(ret));
        return ret;
    }

    printf(_("Profile ID: %s\n"), profile_id);
    printf(_("Enabled features:"));

    if (features == NULL || features[0] == NULL) {
        printf(_(" None\n"));
    } else {
        printf("\n");
        for (i = 0; features[i] != NULL; i++) {
            printf("- %s\n", features[i]);
        }
    }

    free(profile_id);
    authselect_array_free(features);

    return EOK;
}

static errno_t check(struct cli_cmdline *cmdline)
{
    bool is_valid;
    errno_t ret;

    ret = cli_tool_popt(cmdline, NULL, CLI_TOOL_OPT_OPTIONAL, NULL, NULL);
    if (ret != EOK) {
        ERROR("Unable to parse command arguments");
        return ret;
    }

    ret = authselect_validate_configuration(&is_valid);
    if (ret != EOK && ret != ENOENT) {
        ERROR("Unable to test current configuration [%d]: %s",
              ret, strerror(ret));

        return ret;
    }

    if (!is_valid) {
        puts(_("Current configuration is not valid. "
               "It was probably modified outside authselect."));
        return EBADF;
    }

    puts(_("Current configuration is valid."));

    /* EOK = existing configuration is valid,
     * ENOENT = non-existing configuration is valid */
    return ret;
}

static errno_t list(struct cli_cmdline *cmdline)
{
    struct authselect_profile *profile;
    char **profiles;
    errno_t ret;
    int maxlen;
    int i;

    ret = cli_tool_popt(cmdline, NULL, CLI_TOOL_OPT_OPTIONAL, NULL, NULL);
    if (ret != EOK) {
        ERROR("Unable to parse command arguments");
        return ret;
    }

    profiles = authselect_list();
    if (profiles == NULL) {
        ERROR("Unable to get profile list!");
        return ENOMEM;
    }

    maxlen = list_max_length(profiles);

    for (i = 0; profiles[i] != NULL; i++) {
        ret = authselect_profile(profiles[i], &profile);
        if (ret != EOK) {
            ERROR("Unable to get profile information [%d]: %s",
                  ret, strerror(ret));
            goto done;
        }

        printf("- %-*s\t %s\n", maxlen, profiles[i],
               authselect_profile_name(profile));

        authselect_profile_free(profile);
    }

    ret = EOK;

done:
    authselect_array_free(profiles);
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

    ret = authselect_profile(profile_id, &profile);
    if (ret != EOK) {
        ERROR("Unable to get profile information [%d]: %s",
              ret, strerror(ret));
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
    const char **features;
    const char *content;
    const char *path;
    int print_all = 1;
    int print_nsswitch = 0;
    int print_systemauth = 0;
    int print_passwordauth = 0;
    int print_smartcardauth = 0;
    int print_fingerprintauth = 0;
    int print_postlogin = 0;
    int print_dconfdb = 0;
    int print_dconflock = 0;
    errno_t ret;
    int i;

    struct poptOption options[] = {
        {"all", 'a', POPT_ARG_VAL, &print_all, 1, _("Print content of all files"), NULL },
        {"nsswitch", 'n', POPT_ARG_VAL, &print_nsswitch, 1, _("Print nsswitch.conf content"), NULL },
        {"system-auth", 's', POPT_ARG_VAL, &print_systemauth, 1, _("Print system-auth content"), NULL },
        {"password-auth", 'p', POPT_ARG_VAL, &print_passwordauth, 1, _("Print password-auth content"), NULL },
        {"smartcard-auth", 'c', POPT_ARG_VAL, &print_smartcardauth, 1, _("Print smartcard-auth content"), NULL },
        {"fingerprint-auth", 'f', POPT_ARG_VAL, &print_fingerprintauth, 1, _("Print fingerprint-auth content"), NULL },
        {"postlogin", 'o', POPT_ARG_VAL, &print_postlogin, 1, _("Print postlogin content"), NULL },
        {"dconf-db", 'd', POPT_ARG_VAL, &print_dconfdb, 1, _("Print dconf database content"), NULL },
        {"dconf-lock", 'l', POPT_ARG_VAL, &print_dconflock, 1, _("Print dconf lock content"), NULL },
        POPT_TABLEEND
        };

    struct {
        const char * (*content_fn)(const struct authselect_files *);
        const char * (*path_fn)(void);
        int *enabled;
    } generated[] = {
        {authselect_files_nsswitch, authselect_path_nsswitch, &print_nsswitch},
        {authselect_files_systemauth, authselect_path_systemauth, &print_systemauth},
        {authselect_files_passwordauth, authselect_path_passwordauth, &print_passwordauth},
        {authselect_files_smartcardauth, authselect_path_smartcardauth, &print_smartcardauth},
        {authselect_files_fingerprintauth, authselect_path_fingerprintauth, &print_fingerprintauth},
        {authselect_files_postlogin, authselect_path_postlogin, &print_postlogin},
        {authselect_files_dconf_db, authselect_path_dconf_db, &print_dconfdb},
        {authselect_files_dconf_lock, authselect_path_dconf_lock, &print_dconflock},
        {NULL, NULL, NULL}
    };

    ret = parse_profile_options(cmdline, options, &profile_id, &features);
    if (ret != EOK) {
        return ret;
    }

    ret = authselect_files(profile_id, features, &files);
    if (ret != EOK) {
        ERROR("Unable to get generated content [%d]: %s", ret, strerror(ret));
        return ret;
    }

    for (i = 0; generated[i].content_fn != NULL; i++) {
        if (*generated[i].enabled == 1) {
            print_all = 0;
        }
    }

    for (i = 0; generated[i].content_fn != NULL; i++) {
        if (!print_all && *generated[i].enabled == 0) {
            continue;
        }

        path = generated[i].path_fn();
        content = generated[i].content_fn(files);

        if (content == NULL) {
            printf(_("File %s: Empty\n\n"), path);
        } else {
            printf(_("File %s:\n%s\n\n"), path, content);
        }
    }

    return EOK;
}

static errno_t enable(struct cli_cmdline *cmdline)
{
    const char *feature;
    errno_t ret;

    ret = cli_tool_popt_ex(cmdline, NULL, CLI_TOOL_OPT_OPTIONAL,
                           NULL, NULL, "FEATURE", _("Feature to enable."),
                           &feature, false, NULL);
    if (ret != EOK) {
        ERROR("Unable to parse command arguments");
        return ret;
    }

    ret = authselect_feature_enable(feature);
    if (ret != EOK) {
        fprintf(stderr, _("Unable to enable feature [%d]: %s\n"),
                ret, strerror(ret));
        return ret;
    }

    return EOK;
}

static errno_t disable(struct cli_cmdline *cmdline)
{
    const char *feature;
    errno_t ret;

    ret = cli_tool_popt_ex(cmdline, NULL, CLI_TOOL_OPT_OPTIONAL,
                           NULL, NULL, "FEATURE", _("Feature to disable."),
                           &feature, false, NULL);
    if (ret != EOK) {
        ERROR("Unable to parse command arguments");
        return ret;
    }

    ret = authselect_feature_disable(feature);
    if (ret != EOK) {
        fprintf(stderr, _("Unable to disable feature [%d]: %s\n"),
                ret, strerror(ret));
        return ret;
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
        CLI_TOOL_COMMAND("enable-feature", "Enable feature in currently selected profile", enable),
        CLI_TOOL_COMMAND("disable-feature", "Disable feature in currently selected profile", disable),
        CLI_TOOL_LAST
    };

    ret = setup_gettext();
    if (ret != EOK) {
        fprintf(stderr, _("Unable to setup gettext!\n"));
        return 1;
    }

    uid = getuid();
    if (uid != 0) {
        fprintf(stderr, _("Authselect can only be run as root!\n"));
        return 1;
    }

    return cli_tool_main(argc, argv, commands, NULL);
}
