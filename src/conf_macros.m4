dnl Add new --with option.
dnl
dnl Arg 1: option-name
dnl Arg 2: variable_name
dnl Arg 3: output variable name
dnl Arg 4: value-type
dnl Arg 5: help string
dnl Arg 6: default value
dnl
AC_DEFUN([CONFIGURABLE_VALUE], [
    AC_ARG_WITH([$1], [AC_HELP_STRING([--with-$1=$4], [$5 [$6]])])
    
    $3="$6"
    if test x"$with_$2" != x; then
        $3=$with_$2
    fi
    
    AC_SUBST($3)
])

CONFIGURABLE_VALUE(config-dir, config_dir, AUTHSELECT_CONFIG_DIR, DIR,
                   [Path to the directory where authselect stores its configuration],
                   $sysconfdir/authselect)

CONFIGURABLE_VALUE(profile-dir, profile_dir, AUTHSELECT_PROFILE_DIR, DIR,
                   [Path to the directory where are stored authselect profiles],
                   $datarootdir/authselect)
                   
CONFIGURABLE_VALUE(pam-dir, pam_dir, AUTHSELECT_PAM_DIR, DIR,
                   [Path to the pam.d directory where generated pam stacks will be stored],
                   $sysconfdir/pam.d)

CONFIGURABLE_VALUE(nsswitch-conf, nsswitch_conf, AUTHSELECT_NSSWITCH_CONF, PATH,
                   [Path to the nsswitch.conf file],
                   $sysconfdir/nsswitch.conf)

CONFIGURABLE_VALUE(dconf-dir, dconf_dir, AUTHSELECT_DCONF_DIR, DIR,
                   [Path to the dconf database directory to store gdm options],
                   $sysconfdir/dconf/db/distro.d)

CONFIGURABLE_VALUE(dconf-file, dconf_file, AUTHSELECT_DCONF_FILE, NAME,
                   [Name of authselect dconf file to be stored in dconf-db],
                   20-authselect)
