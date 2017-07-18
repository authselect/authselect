AM_CFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/src/lib \
    -ggdb3 \
    $(NULL)

include_HEADERS = \
    $(top_srcdir)/include/authselect.h \
    $(NULL)
    
noinst_HEADERS = \
    ../common/common.h \
    authselect_private.h \
    authselect_util.h \
    authselect_files.h \
    $(NULL)

lib_LTLIBRARIES = \
    libauthselect.la \
    $(NULL)

libauthselect_la_SOURCES = \
	../common/malloc.c \
	../common/debug.c \
    authselect.c \
    authselect_dir.c \
    authselect_profile.c \
    authselect_files.c \
    authselect_conf.c \
    authselect_check.c \
    authselect_activate.c \
    authselect_util.c \
    $(NULL)
libauthselect_la_CFLAGS = \
    $(AM_CFLAGS) \
    -DAUTHSELECT_CONFIG_DIR=\"@AUTHSELECT_CONFIG_DIR@\" \
    -DAUTHSELECT_PROFILE_DIR=\"@AUTHSELECT_PROFILE_DIR@\" \
    -DAUTHSELECT_VENDOR_DIR=\"@AUTHSELECT_VENDOR_DIR@\" \
    -DAUTHSELECT_CUSTOM_DIR=\"@AUTHSELECT_CUSTOM_DIR@\" \
    -DAUTHSELECT_PAM_DIR=\"@AUTHSELECT_PAM_DIR@\" \
    -DAUTHSELECT_NSSWITCH_CONF=\"@AUTHSELECT_NSSWITCH_CONF@\" \
    -DAUTHSELECT_DCONF_FILE=\"@AUTHSELECT_DCONF_FILE@\" \
    $(NULL)