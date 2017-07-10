AM_CFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/src/cli \
    $(NULL)

bin_PROGRAMS = \
    authselect \
    $(NULL)

authselect_SOURCES = \
    main.c \
    $(NULL)
authselect_LDADD = \
	$(top_builddir)/src/lib/libauthselect.la \
    $(NULL)
