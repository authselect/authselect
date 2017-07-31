MKDIR_P = @MKDIR_P@

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
    ../common/gettext.h \
    authselect_private.h \
    authselect_util.h \
    authselect_files.h \
    $(NULL)

lib_LTLIBRARIES = \
    libauthselect.la \
    $(NULL)

authselect_config_dir=@AUTHSELECT_CONFIG_DIR@
authselect_profile_dir=@AUTHSELECT_PROFILE_DIR@
authselect_default_dir=$(authselect_profile_dir)/default
authselect_vendor_dir=$(authselect_profile_dir)/vendor
authselect_custom_dir=$(authselect_profile_dir)/custom
authselect_pam_dir=@AUTHSELECT_PAM_DIR@
authselect_nsswitch_conf=@AUTHSELECT_NSSWITCH_CONF@
authselect_dconf_file=@AUTHSELECT_DCONF_FILE@

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
    -DAUTHSELECT_CONFIG_DIR=\"$(authselect_config_dir)\" \
    -DAUTHSELECT_PROFILE_DIR=\"$(authselect_profile_dir)\" \
    -DAUTHSELECT_PAM_DIR=\"$(authselect_pam_dir)\" \
    -DAUTHSELECT_NSSWITCH_CONF=\"$(authselect_nsswitch_conf)\" \
    -DAUTHSELECT_DCONF_FILE=\"$(authselect_dconf_file)\" \
    $(NULL)

install-exec-hook:
	$(MKDIR_P)  $(DESTDIR)$/$(authselect_config_dir)
	$(MKDIR_P)  $(DESTDIR)$/$(authselect_profile_dir)
	$(MKDIR_P)  $(DESTDIR)$/$(authselect_default_dir)
	$(MKDIR_P)  $(DESTDIR)$/$(authselect_vendor_dir)
	$(MKDIR_P)  $(DESTDIR)$/$(authselect_custom_dir)
	$(MKDIR_P)  $(DESTDIR)$/$(authselect_pam_dir)
	mv $(top_srcdir)/profiles/* $(DESTDIR)$/$(authselect_default_dir)
	$(NULL)
