MKDIR_P = @MKDIR_P@

AM_CFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/src \
    $(NULL)

include_HEADERS = \
    $(top_srcdir)/include/authselect.h \
    $(NULL)
    
noinst_HEADERS = \
    constants.h \
    paths.h \
    files/files.h \
    profiles/profiles.h \
    util/file.h \
    util/string_array.h \
    util/string.h \
    util/template.h \
    util/textfile.h \
    util/util.h \
    $(NULL)

lib_LTLIBRARIES = \
    libauthselect.la \
    $(NULL)

authselect_config_dir=@AUTHSELECT_CONFIG_DIR@
authselect_profile_dir=@AUTHSELECT_PROFILE_DIR@
authselect_vendor_dir=@AUTHSELECT_VENDOR_DIR@
authselect_custom_dir=@AUTHSELECT_CUSTOM_DIR@
authselect_pam_dir=@AUTHSELECT_PAM_DIR@
authselect_nsswitch_conf=@AUTHSELECT_NSSWITCH_CONF@
authselect_dconf_dir=@AUTHSELECT_DCONF_DIR@
authselect_dconf_file=@AUTHSELECT_DCONF_FILE@

libauthselect_la_SOURCES = \
    authselect.c \
    authselect_profile.c \
    authselect_files.c \
    authselect_paths.c \
    files/config.c \
    files/symlinks.c \
    files/system.c \
    profiles/activate.c \
    profiles/custom.c \
    profiles/list.c \
    profiles/read.c \
    util/file.c \
    util/string_array.c \
    util/string.c \
    util/template.c \
    util/textfile.c \
    $(NULL)
libauthselect_la_LIBADD = \
    $(top_builddir)/src/common/libcommon.la \
    $(NULL)
libauthselect_la_CFLAGS = \
    $(AM_CFLAGS) \
    -DAUTHSELECT_CONFIG_DIR=\"$(authselect_config_dir)\" \
    -DAUTHSELECT_PROFILE_DIR=\"$(authselect_profile_dir)\" \
    -DAUTHSELECT_VENDOR_DIR=\"$(authselect_vendor_dir)\" \
    -DAUTHSELECT_CUSTOM_DIR=\"$(authselect_custom_dir)\" \
    -DAUTHSELECT_PAM_DIR=\"$(authselect_pam_dir)\" \
    -DAUTHSELECT_NSSWITCH_CONF=\"$(authselect_nsswitch_conf)\" \
    -DAUTHSELECT_DCONF_DIR=\"$(authselect_dconf_dir)\" \
    -DAUTHSELECT_DCONF_FILE=\"$(authselect_dconf_file)\" \
    $(NULL)
libauthselect_la_LDFLAGS = \
    -version-info 0:1:0
