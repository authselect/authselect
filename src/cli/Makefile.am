AM_CFLAGS = \
   -I$(top_srcdir)/include \
   -I$(top_srcdir)/src/cli \
   -DLOCALEDIR='"$(localedir)"' \
   -ggdb3 \
    $(NULL)

bin_PROGRAMS = \
    authselect \
    $(NULL)

authselect_SOURCES = \
    ../common/malloc.c \
    ../common/debug.c \
    cli_tool.c \
    main.c \
    $(NULL)
authselect_LDADD = \
    $(top_builddir)/src/lib/libauthselect.la \
    $(POPT_LIBS) \
    $(NULL)
authsekect_CFLAGS = \
    $(AM_CFLAGS) \
    $(POPT_CFLAGS) \
    $(NULL)
