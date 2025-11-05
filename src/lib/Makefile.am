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
    util/dir.h \
    util/file.h \
    util/selinux.h \
    util/string_array.h \
    util/string.h \
    util/template.h \
    util/evaluator.h \
    util/textfile.h \
    util/util.h \
    $(NULL)

dist_noinst_DATA = \
    authselect.exports \
    $(NULL)

lib_LTLIBRARIES = \
    libauthselect.la \
    $(NULL)

authselect_config_dir=@AUTHSELECT_CONFIG_DIR@
authselect_state_dir=@AUTHSELECT_STATE_DIR@
authselect_profile_dir=@AUTHSELECT_PROFILE_DIR@
authselect_custom_dir=@AUTHSELECT_CUSTOM_DIR@
authselect_pam_dir=@AUTHSELECT_PAM_DIR@
authselect_nsswitch_conf=@AUTHSELECT_NSSWITCH_CONF@
authselect_dconf_dir=@AUTHSELECT_DCONF_DIR@
authselect_dconf_file=@AUTHSELECT_DCONF_FILE@
authselect_dconf_bin=@AUTHSELECT_DCONF_BIN@
authselect_backup_dir=@AUTHSELECT_BACKUP_DIR@

libauthselect_la_SOURCES = \
    authselect.c \
    authselect_backup.c \
    authselect_profile.c \
    authselect_files.c \
    authselect_paths.c \
    files/config.c \
    files/nsswitch.c \
    files/symlinks.c \
    files/system.c \
    profiles/activate.c \
    profiles/custom.c \
    profiles/list.c \
    profiles/read.c \
    util/dir.c \
    util/file.c \
    util/selinux.c \
    util/string_array.c \
    util/string.c \
    util/template.c \
    util/evaluator.c \
    util/textfile.c \
    $(NULL)
libauthselect_la_LIBADD = \
    $(top_builddir)/src/common/libcommon.la \
    $(SELINUX_LIBS) \
    $(NULL)
libauthselect_la_CFLAGS = \
    $(AM_CFLAGS) \
    -DAUTHSELECT_CONFIG_DIR=\"$(authselect_config_dir)\" \
    -DAUTHSELECT_STATE_DIR=\"$(authselect_state_dir)\" \
    -DAUTHSELECT_PROFILE_DIR=\"$(authselect_profile_dir)\" \
    -DAUTHSELECT_CUSTOM_DIR=\"$(authselect_custom_dir)\" \
    -DAUTHSELECT_PAM_DIR=\"$(authselect_pam_dir)\" \
    -DAUTHSELECT_NSSWITCH_CONF=\"$(authselect_nsswitch_conf)\" \
    -DAUTHSELECT_DCONF_DIR=\"$(authselect_dconf_dir)\" \
    -DAUTHSELECT_DCONF_FILE=\"$(authselect_dconf_file)\" \
    -DAUTHSELECT_DCONF_BIN=\"$(authselect_dconf_bin)\" \
    -DAUTHSELECT_BACKUP_DIR=\"$(authselect_backup_dir)\" \
    $(NULL)
libauthselect_la_LDFLAGS = \
    -Wl,--version-script=$(srcdir)/authselect.exports \
    -version-info 7:0:2

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
    authselect.pc \
    $(NULL)
