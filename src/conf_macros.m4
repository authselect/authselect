AC_DEFUN([WITH_PROFILE_DIR], [
    AC_ARG_WITH(
        [profile-dir],
        [AC_HELP_STRING(
            [--with-profile-dir=DIR],
            [Path to the read-only profile directory for 
             profiles shipped with the tool [/usr/lib/sssd]]
        )]
    )
    
    profile_dir="/home/pbrezina/profiles/default"
    if test x"$with_profile_dir" != x; then
        profile_dir=$with_profile_dir
    fi
    AC_SUBST(profile_dir)
    AC_DEFINE_UNQUOTED(AUTHSELECT_PROFILE_DIR,
                       "$profile_dir",
                       [Path to the default profiles directory])
])

AC_DEFUN([WITH_VENDOR_DIR], [
    AC_ARG_WITH(
        [vendor-dir],
        [AC_HELP_STRING(
            [--with-vendor-dir=DIR],
            [Path to the read-only profile directory for 
             profiles shipped by system vendor [/usr/lib/sssd]]
        )]
    )
    
    vendor_dir="/home/pbrezina/profiles/vendor"
    if test x"$with_vendor_dir" != x; then
        vendor_dir=$with_vendor_dir
    fi
    AC_SUBST(vendor_dir)
    AC_DEFINE_UNQUOTED(AUTHSELECT_VENDOR_DIR,
                       "$vendor_dir",
                       [Path to the vendor profiles directory])
])

AC_DEFUN([WITH_CUSTOM_DIR], [
    AC_ARG_WITH(
        [custom-dir],
        [AC_HELP_STRING(
            [--with-custom-dir=DIR],
            [Path to the profile directory for custom 
             profiles created by administrator [/usr/lib/sssd]]
        )]
    )
    
    custom_dir="/home/pbrezina/profiles/custom"
    if test x"$with_custom_dir" != x; then
        vendor_dir=$with_custom_dir
    fi
    AC_SUBST(custom_dir)
    AC_DEFINE_UNQUOTED(AUTHSELECT_CUSTOM_DIR,
                       "$custom_dir",
                       [Path to the custom profiles directory])
])
