authselect_profile_dir=@AUTHSELECT_PROFILE_DIR@

# The shipped profiles
profile_localdir = $(authselect_profile_dir)/local
dist_profile_local_DATA = \
    $(top_srcdir)/profiles/local/nsswitch.conf \
    $(top_srcdir)/profiles/local/password-auth \
    $(top_srcdir)/profiles/local/postlogin \
    $(top_srcdir)/profiles/local/README \
    $(top_srcdir)/profiles/local/REQUIREMENTS \
    $(top_srcdir)/profiles/local/smartcard-auth \
    $(top_srcdir)/profiles/local/switchable-auth \
    $(top_srcdir)/profiles/local/system-auth \
    $(top_srcdir)/profiles/local/fingerprint-auth \
    $(top_srcdir)/profiles/local/dconf-db \
    $(top_srcdir)/profiles/local/dconf-locks \
    $(NULL)

if WITH_NIS_PROFILE
profile_nisdir = $(authselect_profile_dir)/nis
dist_profile_nis_DATA = \
    $(top_srcdir)/profiles/nis/nsswitch.conf \
    $(top_srcdir)/profiles/nis/password-auth \
    $(top_srcdir)/profiles/nis/postlogin \
    $(top_srcdir)/profiles/nis/README \
    $(top_srcdir)/profiles/nis/REQUIREMENTS \
    $(top_srcdir)/profiles/nis/smartcard-auth \
    $(top_srcdir)/profiles/nis/switchable-auth \
    $(top_srcdir)/profiles/nis/system-auth \
    $(top_srcdir)/profiles/nis/fingerprint-auth \
    $(top_srcdir)/profiles/nis/dconf-db \
    $(top_srcdir)/profiles/nis/dconf-locks \
    $(NULL)
endif

profile_sssddir = $(authselect_profile_dir)/sssd
dist_profile_sssd_DATA = \
    $(top_srcdir)/profiles/sssd/nsswitch.conf \
    $(top_srcdir)/profiles/sssd/password-auth \
    $(top_srcdir)/profiles/sssd/postlogin \
    $(top_srcdir)/profiles/sssd/README \
    $(top_srcdir)/profiles/sssd/REQUIREMENTS \
    $(top_srcdir)/profiles/sssd/smartcard-auth \
    $(top_srcdir)/profiles/sssd/switchable-auth \
    $(top_srcdir)/profiles/sssd/system-auth \
    $(top_srcdir)/profiles/sssd/fingerprint-auth \
    $(top_srcdir)/profiles/sssd/dconf-db \
    $(top_srcdir)/profiles/sssd/dconf-locks \
    $(NULL)

profile_winbinddir = $(authselect_profile_dir)/winbind
dist_profile_winbind_DATA = \
    $(top_srcdir)/profiles/winbind/nsswitch.conf \
    $(top_srcdir)/profiles/winbind/password-auth \
    $(top_srcdir)/profiles/winbind/postlogin \
    $(top_srcdir)/profiles/winbind/README \
    $(top_srcdir)/profiles/winbind/REQUIREMENTS \
    $(top_srcdir)/profiles/winbind/smartcard-auth \
    $(top_srcdir)/profiles/winbind/switchable-auth \
    $(top_srcdir)/profiles/winbind/system-auth \
    $(top_srcdir)/profiles/winbind/fingerprint-auth \
    $(top_srcdir)/profiles/winbind/dconf-db \
    $(top_srcdir)/profiles/winbind/dconf-locks \
    $(NULL)
