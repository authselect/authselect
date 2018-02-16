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

authselect_SOURCES = \
    cli_tool.c \
    main.c \
    $(NULL)
authselect_LDADD = \
    $(top_builddir)/src/common/libcommon.la \
    $(top_builddir)/src/lib/libauthselect.la \
    $(POPT_LIBS) \
    $(NULL)
authsekect_CFLAGS = \
    $(AM_CFLAGS) \
    $(POPT_CFLAGS) \
    $(NULL)
