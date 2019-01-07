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

#ifndef _CLI_TOOL_H_
#define _CLI_TOOL_H_

#include <stdbool.h>
#include <popt.h>

struct cli_cmdline;

typedef errno_t
(*cli_route_fn)(struct cli_cmdline *cmdline);

#define CLI_CMD_NONE         0x0000
#define CLI_CMD_REQUIRE_ROOT 0x0001

#define CLI_TOOL_COMMAND(cmd, msg, flags, fn)     {cmd, _(msg), (flags), fn}
#define CLI_TOOL_COMMAND_NOMSG(cmd, flags, fn)    {cmd, NULL, (flags), fn}
#define CLI_TOOL_DELIMITER(message)        {"", _(message), CLI_CMD_NONE, NULL}
#define CLI_TOOL_LAST                      {NULL, NULL, CLI_CMD_NONE, NULL}

struct cli_cmdline {
    const char *exec; /* argv[0] */
    const char *command; /* command name */
    int argc; /* rest of arguments */
    const char **argv;
};

struct cli_route_cmd {
    const char *command;
    const char *description;
    uint32_t flags;
    cli_route_fn fn;
};

void cli_tool_usage(const char *tool_name, struct cli_route_cmd *commands);

typedef errno_t (*cli_popt_fn)(poptContext pc, char option, void *pvt);

enum cli_tool_opt {
    CLI_TOOL_OPT_REQUIRED,
    CLI_TOOL_OPT_OPTIONAL
};

errno_t cli_tool_popt_ex(struct cli_cmdline *cmdline,
                         struct poptOption *options,
                         enum cli_tool_opt require_option,
                         cli_popt_fn popt_fn,
                         void *popt_fn_pvt,
                         const char *fopt_name,
                         const char *fopt_help,
                         const char **_fopt,
                         bool allow_more_free_opts,
                         bool *_opt_set);

errno_t cli_tool_popt(struct cli_cmdline *cmdline,
                      struct poptOption *options,
                      enum cli_tool_opt require_option,
                      cli_popt_fn popt_fn,
                      void *popt_fn_pvt);

int cli_tool_main(int argc, const char **argv,
                  struct cli_route_cmd *commands,
                  void *pvt);

#endif /* _CLI_TOOL_H_ */
