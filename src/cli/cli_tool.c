/*
    Authors:
        Pavel Březina <pbrezina@redhat.com>

    Copyright (C) 2015 Red Hat

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
#include <string.h>
#include <unistd.h>
#include <popt.h>

#include "common/common.h"
#include "cli/cli_tool.h"
#include "authselect.h"

bool enable_trace;
bool enable_warning;
bool enable_debug;

void
print_debug(void *pvt,
            enum authselect_debug level,
            const char *file,
            unsigned long line,
            const char *function,
            const char *msg)
{
    const char *category = "unknown";

    switch (level) {
    case AUTHSELECT_INFO:
        if (!enable_trace) {
            return;
        }
        category = "info";
        break;
    case AUTHSELECT_WARNING:
        if (!enable_warning) {
            return;
        }
        category = "warn";
        break;
    case AUTHSELECT_ERROR:
        category = "error";
        break;
    }

    if (!enable_debug) {
        fprintf(stderr, "[%s] %s\n", category, msg);
        return;
    }

    fprintf(stderr, "[%s] [%s] %s\n", category, function, msg);
}

static void cli_tool_print_common_opts(int min_len)
{
    fprintf(stderr, _("Common options:\n"));
    fprintf(stderr, "  %-*s\t %s\n", min_len, "--debug",
                    _("Print error messages"));
    fprintf(stderr, "  %-*s\t %s\n", min_len, "--trace",
                    _("Print trace messages"));
    fprintf(stderr, "  %-*s\t %s\n", min_len, "--warn",
                    _("Print warning messages"));
    fprintf(stderr, "\n");
    fprintf(stderr, _("Help options:\n"));
    fprintf(stderr, "  %-*s\t %s\n", min_len, "-?, --help",
                    _("Show this for a command"));
    fprintf(stderr, "  %-*s\t %s\n", min_len, "--usage",
                    _("Show brief usage message for a command"));
}

static struct poptOption *cli_tool_common_opts_table(void)
{
    static struct poptOption options[] = {
        {"debug", '\0', POPT_ARG_NONE | POPT_ARGFLAG_STRIP, NULL, 'd', "Print more verbose debugging information", NULL },
        {"trace", '\0', POPT_ARG_NONE | POPT_ARGFLAG_STRIP, NULL, 't', "Print trace messages", NULL },
        {"warn", '\0', POPT_ARG_NONE | POPT_ARGFLAG_STRIP, NULL, 'w', "Print warning messages", NULL },
        POPT_TABLEEND
    };

    return options;
}

static void cli_tool_common_opts(int *argc, const char **argv)
{
    poptContext pc;
    struct poptOption *options;
    int orig_argc = *argc;
    int opt;

    options = cli_tool_common_opts_table();

    pc = poptGetContext(argv[0], orig_argc, argv, options, 0);
    while ((opt = poptGetNextOpt(pc)) != -1) {
        switch (opt) {
        case 'd':
            enable_debug = true;
            break;
        case 't':
            enable_trace = true;
            break;
        case 'w':
            enable_warning = true;
            break;
        default:
            break;
        }
    }

    set_debug_fn(print_debug, NULL);
    authselect_set_debug_fn(print_debug, NULL);

    /* Strip common options from arguments. We will discard_const here,
     * since it is not worth the trouble to convert it back and forth. */
    *argc = poptStrippedArgv(pc, orig_argc, (char **)argv);

    poptFreeContext(pc);
}

static bool cli_tool_is_delimiter(struct cli_route_cmd *command)
{
    if (command->command != NULL && command->command[0] == '\0') {
        return true;
    }

    return false;
}

static size_t cli_tool_max_length(struct cli_route_cmd *commands)
{
    size_t max = 0;
    size_t len;
    int i;

    for (i = 0; commands[i].command != NULL; i++) {
        if (cli_tool_is_delimiter(&commands[i])) {
            continue;
        }

        len = strlen(commands[i].command);
        if (max < len) {
            max = len;
        }
    }

    return max;
}

void cli_tool_usage(const char *tool_name, struct cli_route_cmd *commands)
{
    int min_len;
    int i;

    fprintf(stderr, _("Usage:\n%s COMMAND COMMAND-ARGS\n\n"), tool_name);
    fprintf(stderr, _("Available commands:\n"));

    min_len = cli_tool_max_length(commands);

    for (i = 0; commands[i].command != NULL; i++) {
        if (cli_tool_is_delimiter(&commands[i])) {
            fprintf(stderr, "\n%s\n", commands[i].description);
            continue;
        }

        if (commands[i].description == NULL) {
            fprintf(stderr, "- %40s\n", commands[i].command);
        } else {
            fprintf(stderr, "- %-*s\t %s\n",
                    min_len, commands[i].command, commands[i].description);
        }
    }

    fprintf(stderr, _("\n"));
    cli_tool_print_common_opts(min_len);
}

static bool
cli_tool_check_root_access(uint32_t flags,
                           const char *command,
                           int argc,
                           const char **argv)
{
    uid_t uid;
    int i;

    if (!(flags & CLI_CMD_REQUIRE_ROOT)) {
        return true;
    }

    /* If help was requested, we allow the access. */
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-?") == 0) {
            return true;
        }

        if (strcmp(argv[i], "--help") == 0) {
            return true;
        }

        if (strcmp(argv[i], "--usage") == 0) {
            return true;
        }
    }

    uid = getuid();
    if (uid != 0) {
        fprintf(stderr, _("Authselect command '%s' can only be run as root!\n"),
                command);
        return false;
    }

    return true;
}

errno_t cli_tool_route(int argc, const char **argv,
                       struct cli_route_cmd *commands)
{
    struct cli_cmdline cmdline;
    const char *cmd;
    bool bret;
    int i;

    if (commands == NULL) {
        ERROR("Bug: commands can't be NULL!\n");
        return EINVAL;
    }

    if (argc < 2) {
        cli_tool_usage(argv[0], commands);
        return EINVAL;
    }

    cmd = argv[1];
    for (i = 0; commands[i].command != NULL; i++) {
        if (cli_tool_is_delimiter(&commands[i])) {
            continue;
        }

        if (strcmp(commands[i].command, cmd) == 0) {
            bret = cli_tool_check_root_access(commands[i].flags,
                                              commands[i].command,
                                              argc, argv);
            if (!bret) {
                return EACCES;
            }

            cmdline.exec = argv[0];
            cmdline.command = argv[1];
            cmdline.argc = argc - 2;
            cmdline.argv = argv + 2;

            return commands[i].fn(&cmdline);
        }
    }

    cli_tool_usage(argv[0], commands);

    return EINVAL;
}

static struct poptOption *nonnull_popt_table(struct poptOption *options)
{
    static struct poptOption empty[] = {
        POPT_TABLEEND
    };

    if (options == NULL) {
        return empty;
    }

    return options;
}

errno_t cli_tool_popt_ex(struct cli_cmdline *cmdline,
                         struct poptOption *options,
                         enum cli_tool_opt require_option,
                         cli_popt_fn popt_fn,
                         void *popt_fn_pvt,
                         const char *fopt_name,
                         const char *fopt_help,
                         const char **_fopt,
                         bool allow_more_free_opts,
                         bool *_opt_set)
{
    struct poptOption opts_table[] = {
        {NULL, '\0', POPT_ARG_INCLUDE_TABLE, nonnull_popt_table(options), \
         0, _("Command options:"), NULL },
        {NULL, '\0', POPT_ARG_INCLUDE_TABLE, cli_tool_common_opts_table(), \
         0, _("Common options:"), NULL },
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    const char *fopt;
    char *help;
    poptContext pc;
    bool opt_set;
    int ret;

    /* Create help option string. We always need to append command name since
     * we use POPT_CONTEXT_KEEP_FIRST. */
    if (fopt_name == NULL) {
        help = format("%s %s %s", cmdline->exec,
                      cmdline->command, _("[OPTIONS...]"));
    } else {
        help = format("%s %s %s %s", cmdline->exec,
                      cmdline->command, fopt_name, _("[OPTIONS...]"));
    }
    if (help == NULL) {
        ERROR("Out of memory!");
        return ENOMEM;
    }

    /* Create popt context. This function is supposed to be called on
     * command argv which does not contain executable (argv[0]), therefore
     * we need to use KEEP_FIRST that ensures argv[0] is also processed. */
    pc = poptGetContext(cmdline->exec, cmdline->argc, cmdline->argv,
                        opts_table, POPT_CONTEXT_KEEP_FIRST);

    poptSetOtherOptionHelp(pc, help);

    /* Parse options. Invoke custom function if provided. If no parsing
     * function is provided, print error on unknown option. */
    while ((ret = poptGetNextOpt(pc)) != -1) {
        if (popt_fn != NULL) {
            ret = popt_fn(pc, ret, popt_fn_pvt);
            if (ret != EOK) {
                goto done;
            }
        } else {
            fprintf(stderr, _("Invalid option %s: %s\n\n"),
                    poptBadOption(pc, 0), poptStrerror(ret));
            poptPrintHelp(pc, stderr, 0);
            ret = EINVAL;
            goto done;
        }
    }

    /* Parse free option which is always required if requested. */
    fopt = poptGetArg(pc);
    if (_fopt != NULL) {
        if (fopt == NULL) {
            fprintf(stderr, _("Missing option: %s\n\n"), fopt_help);
            poptPrintHelp(pc, stderr, 0);
            ret = EINVAL;
            goto done;
        }

        if (!allow_more_free_opts) {
            /* No more arguments expected.
             * If something follows it is an error. */
            if (poptGetArg(pc)) {
                fprintf(stderr, _("Only one free argument is expected!\n\n"));
                poptPrintHelp(pc, stderr, 0);
                ret = EINVAL;
                goto done;
            }
        }

        *_fopt = fopt;
    } else if (_fopt == NULL && fopt != NULL) {
        /* Unexpected free argument. */
        fprintf(stderr, _("Unexpected parameter: %s\n\n"), fopt);
        poptPrintHelp(pc, stderr, 0);
        ret = EINVAL;
        goto done;
    }

    opt_set = true;
    if ((_fopt != NULL && cmdline->argc < 2) || cmdline->argc < 1) {
        opt_set = false;

        /* If at least one option is required and not provided, print error. */
        if (require_option == CLI_TOOL_OPT_REQUIRED) {
            fprintf(stderr, _("At least one option is required!\n\n"));
            poptPrintHelp(pc, stderr, 0);
            ret = EINVAL;
            goto done;
        }
    }

    if (_opt_set != NULL) {
        *_opt_set = opt_set;
    }

    ret = EOK;

done:
    poptFreeContext(pc);
    free(help);
    return ret;
}

errno_t cli_tool_popt(struct cli_cmdline *cmdline,
                      struct poptOption *options,
                      enum cli_tool_opt require_option,
                      cli_popt_fn popt_fn,
                      void *popt_fn_pvt)
{
    return cli_tool_popt_ex(cmdline, options, require_option,
                            popt_fn, popt_fn_pvt, NULL, NULL,
                            NULL, false, NULL);
}

int cli_tool_main(int argc, const char **argv,
                  struct cli_route_cmd *commands,
                  void *pvt)
{
    errno_t ret;

    cli_tool_common_opts(&argc, argv);

    ret = cli_tool_route(argc, argv, commands);

    switch (ret) {
    case EOK:
        return 0;
    case ENOENT:
        return 2;
    case EBADF:
        return 3;
    case EEXIST:
        return 4;
    case EACCES:
        return 5;
    }

    /* Generic error. */
    return 1;
}
