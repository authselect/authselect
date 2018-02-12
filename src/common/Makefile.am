AM_CFLAGS = \
    -D_GNU_SOURCE \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/src \
    $(NULL)

dist_noinst_HEADERS = \
    common.h \
    gettext.h \
    $(NULL)

noinst_LTLIBRARIES = \
    libcommon.la \
    $(NULL)

libcommon_la_SOURCES = \
    debug.c \
    $(NULL)
libcommon_la_CFLAGS = \
    $(AM_CFLAGS) \
    $(NULL)
