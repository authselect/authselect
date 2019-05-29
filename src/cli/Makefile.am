AM_CFLAGS = \
   -I$(top_srcdir)/include \
   -I$(top_srcdir)/src \
   -DLOCALEDIR='"$(localedir)"' \
    $(NULL)

bin_PROGRAMS = \
    authselect \
    $(NULL)

dist_noinst_HEADERS = \
    cli_tool.h \
    $(NULL)

authselect_backup_dir=@AUTHSELECT_BACKUP_DIR@

authselect_SOURCES = \
    cli_tool.c \
    main.c \
    $(NULL)
authselect_LDADD = \
    $(top_builddir)/src/common/libcommon.la \
    $(top_builddir)/src/lib/libauthselect.la \
    $(POPT_LIBS) \
    $(NULL)
authselect_CFLAGS = \
    $(AM_CFLAGS) \
    $(POPT_CFLAGS) \
    -DAUTHSELECT_BACKUP_DIR=\"$(authselect_backup_dir)\" \
    $(NULL)

# Install bash completion script
completion_dir=@BASH_COMPLETION_DIR@
completiondir = $(completion_dir)
dist_completion_DATA = \
    $(top_srcdir)/src/cli/authselect-completion.sh \
    $(NULL)
