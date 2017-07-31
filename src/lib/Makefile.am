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

# The shipped profiles
profile_sssddir = $(authselect_default_dir)/sssd
dist_profile_sssd_DATA = \
	$(top_srcdir)/profiles/sssd/nsswitch.conf \
	$(top_srcdir)/profiles/sssd/password-auth \
	$(top_srcdir)/profiles/sssd/README \
	$(top_srcdir)/profiles/sssd/system-auth \
	$(NULL)

profile_winbinddir = $(authselect_default_dir)/winbind
dist_profile_winbind_DATA = \
	$(top_srcdir)/profiles/winbind/nsswitch.conf \
	$(top_srcdir)/profiles/winbind/password-auth \
	$(top_srcdir)/profiles/winbind/README \
	$(top_srcdir)/profiles/winbind/system-auth \
	$(NULL)

profile_sssd_fprintdir = $(authselect_default_dir)/sssd+fprint
dist_profile_sssd_fprint_DATA = \
	$(top_srcdir)/profiles/sssd+fprint/nsswitch.conf \
	$(top_srcdir)/profiles/sssd+fprint/password-auth \
	$(top_srcdir)/profiles/sssd+fprint/README \
	$(top_srcdir)/profiles/sssd+fprint/system-auth \
	$(top_srcdir)/profiles/sssd+fprint/fingerprint-auth \
	$(NULL)

profile_winbind_fprintdir = $(authselect_default_dir)/winbind+fprint
dist_profile_winbind_fprint_DATA = \
	$(top_srcdir)/profiles/winbind+fprint/nsswitch.conf \
	$(top_srcdir)/profiles/winbind+fprint/password-auth \
	$(top_srcdir)/profiles/winbind+fprint/README \
	$(top_srcdir)/profiles/winbind+fprint/system-auth \
	$(top_srcdir)/profiles/winbind+fprint/fingerprint-auth \
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
    -DAUTHSELECT_CONFIG_DIR=\"$(authselect_config_dir)\" \
    -DAUTHSELECT_PROFILE_DIR=\"$(authselect_profile_dir)\" \
    -DAUTHSELECT_PAM_DIR=\"$(authselect_pam_dir)\" \
    -DAUTHSELECT_NSSWITCH_CONF=\"$(authselect_nsswitch_conf)\" \
    -DAUTHSELECT_DCONF_FILE=\"$(authselect_dconf_file)\" \
    $(NULL)

install-exec-hook:
	$(MKDIR_P)  $(DESTDIR)$/$(authselect_config_dir)
	$(MKDIR_P)  $(DESTDIR)$/$(authselect_vendor_dir)
	$(MKDIR_P)  $(DESTDIR)$/$(authselect_custom_dir)
