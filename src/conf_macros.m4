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
    AC_ARG_WITH([$1], [AS_HELP_STRING([--with-$1=$4], [$5 [$6]])])

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
                   [Path to the directory where are stored authselect default profiles],
                   $datarootdir/authselect/default)

CONFIGURABLE_VALUE(vendor-dir, vendor_dir, AUTHSELECT_VENDOR_DIR, DIR,
                   [Path to the directory where are stored profiles created by 3rd-party vendors],
                   $datarootdir/authselect/vendor)

CONFIGURABLE_VALUE(custom-dir, custom_dir, AUTHSELECT_CUSTOM_DIR, DIR,
                   [Path to the directory where are stored profiles created by administrator],
                   $sysconfdir/authselect/custom)

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

CONFIGURABLE_VALUE(dconf, dconf, AUTHSELECT_DCONF_BIN, PATH,
                   [Path to the dconf utility],
                   $bindir/dconf)

CONFIGURABLE_VALUE(backup-dir, backup_dir, AUTHSELECT_BACKUP_DIR, DIR,
                   [Directory where configuration backups should be stored],
                   $localstatedir/lib/authselect/backups)

CONFIGURABLE_VALUE(pythonbin, pythonbin, PYTHON_BIN, PATH,
                   [Path to the python interpreter],
                   $bindir/python3)

CONFIGURABLE_VALUE(completion-dir, completion_dir, BASH_COMPLETION_DIR, DIR,
                   [Path to the directory where bash completion script should be stored],
                   $sysconfdir/bash_completion.d)

PKG_CHECK_VAR(PKG_CONFIG_SYSTEMD_SERVICE_DIR, systemd, systemdsystemunitdir, [], [])
if test x"$PKG_CONFIG_SYSTEMD_SERVICE_DIR" = x; then
    PKG_CONFIG_SYSTEMD_SERVICE_DIR="${prefix}/lib/systemd/system"
fi
if test "${PKG_CONFIG_SYSTEMD_SERVICE_DIR:0:${#prefix}}" != "${prefix}"; then
    PKG_CONFIG_SYSTEMD_SERVICE_DIR=${prefix}${PKG_CONFIG_SYSTEMD_SERVICE_DIR}
fi

CONFIGURABLE_VALUE(systemd-service-dir, systemd_service_dir, SYSTEMD_SERVICE_DIR, DIR,
                   [Path to the directory where systemd service files will be installed],
                   $PKG_CONFIG_SYSTEMD_SERVICE_DIR)

AC_ARG_ENABLE(
    [debug-template-regex],
    AS_HELP_STRING(
        [--enable-debug-template-regex],
        [Regular expression matches will be print during template generation]
    )
)

AS_IF([test "x$enable_debug_template_regex" = "xyes"],
    AC_DEFINE_UNQUOTED(
        DEBUG_TEMPLATE_REGEX, 1,
        [Debug template regular expressions]
    )
)

AC_ARG_WITH([nis-profile],
    [AS_HELP_STRING([--with-nis-profile], [Install NIS profile [no]])],
    [], with_nis_profile=no
)
AM_CONDITIONAL([WITH_NIS_PROFILE], [test x$with_nis_profile = xyes])
AC_SUBST(WITH_NIS_PROFILE, 0)
if test x"$with_nis_profile" = xyes; then
    AC_SUBST(WITH_NIS_PROFILE, 1)
fi
