authselect_profile_dir=@AUTHSELECT_PROFILE_DIR@
authselect_default_dir=$(authselect_profile_dir)/default

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
