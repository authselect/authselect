AM_CFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/src \
    $(NULL)

noinst_HEADERS = \
    test_common.h \
    $(NULL)

TESTS = \
    test_util_string_array \
    $(NULL)

check_PROGRAMS = $(TESTS)

test_util_string_array_SOURCES = \
    test_util_string_array.c \
    ../lib/util/string_array.c \
    $(NULL)
test_util_string_array_CFLAGS = \
    $(AM_CFLAGS)
test_util_string_array_LDADD = \
    $(CMOCKA_LIBS) \
    $(top_builddir)/src/common/libcommon.la \
    $(NULL)
