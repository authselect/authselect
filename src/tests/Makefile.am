# Linking tests with sources from other directories creates a problem for
# make distclean, which recursively removes dependencies. When it tries to
# remove a dependency that was already removed as part of cleaning up
# other directory, it fails. Disabling dependency for tests only is a
# fine workaround.
AUTOMAKE_OPTIONS = no-dependencies

AM_CFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/src \
    $(NULL)

noinst_HEADERS = \
    test_common.h \
    $(NULL)

TESTS = \
    test_util_string_array \
    test_util_evaluator \
    test_util_template \
    $(NULL)

check_PROGRAMS = $(TESTS)

test_util_string_array_SOURCES = \
    test_util_string_array.c \
    ../lib/util/string_array.c \
    ../lib/util/string.c \
    $(NULL)
test_util_string_array_CFLAGS = \
    $(AM_CFLAGS)
test_util_string_array_LDADD = \
    $(CMOCKA_LIBS) \
    $(top_builddir)/src/common/libcommon.la \
    $(NULL)

test_util_template_SOURCES = \
    test_util_template.c \
    ../lib/util/file.c \
    ../lib/util/selinux.c \
    ../lib/util/string.c \
    ../lib/util/string_array.c \
    ../lib/util/template.c \
    ../lib/util/evaluator.c \
    ../lib/util/textfile.c \
    $(NULL)
test_util_template_CFLAGS = \
    $(AM_CFLAGS)
test_util_template_LDADD = \
    $(CMOCKA_LIBS) \
    $(SELINUX_LIBS) \
    $(top_builddir)/src/common/libcommon.la \
    $(NULL)

test_util_evaluator_SOURCES = \
    test_util_evaluator.c \
    ../lib/util/string_array.c \
    ../lib/util/string.c \
    $(NULL)
test_util_evaluator_CFLAGS = \
    $(AM_CFLAGS)
test_util_evaluator_LDADD = \
    $(CMOCKA_LIBS) \
    $(top_builddir)/src/common/libcommon.la \
    $(NULL)
