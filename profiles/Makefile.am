authselect_profile_dir=@AUTHSELECT_PROFILE_DIR@
authselect_default_dir=$(authselect_profile_dir)/default

# The shipped profiles
profile_sssddir = $(authselect_default_dir)/sssd
dist_profile_sssd_DATA = \
    $(top_srcdir)/profiles/sssd/nsswitch.conf \
    $(top_srcdir)/profiles/sssd/password-auth \
    $(top_srcdir)/profiles/sssd/postlogin \
    $(top_srcdir)/profiles/sssd/README \
    $(top_srcdir)/profiles/sssd/smartcard-auth \
    $(top_srcdir)/profiles/sssd/system-auth \
    $(top_srcdir)/profiles/sssd/fingerprint-auth \
    $(NULL)

profile_winbinddir = $(authselect_default_dir)/winbind
dist_profile_winbind_DATA = \
    $(top_srcdir)/profiles/winbind/nsswitch.conf \
    $(top_srcdir)/profiles/winbind/password-auth \
    $(top_srcdir)/profiles/winbind/postlogin \
    $(top_srcdir)/profiles/winbind/README \
    $(top_srcdir)/profiles/winbind/system-auth \
    $(top_srcdir)/profiles/winbind/fingerprint-auth \
    $(NULL)
