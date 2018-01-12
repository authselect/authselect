SUBDIRS= \
    po \
    profiles \
    src/common \
    src/lib \
    src/cli \
    $(NULL)

if HAVE_MANPAGES
SUBDIRS += src/man
endif

# Documentation - for now just the license
dist_doc_DATA = \
    COPYING \
    README.md \
    $(NULL)

ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = \
    build/config.rpath \
    rpm/authselect.spec \
    $(NULL)

# Create authselect directories
authselect_config_dir=@AUTHSELECT_CONFIG_DIR@
authselect_vendor_dir=@AUTHSELECT_VENDOR_DIR@
authselect_custom_dir=@AUTHSELECT_CUSTOM_DIR@
authselect_dconf_dir=@AUTHSELECT_DCONF_DIR@
authselect_pam_dir=@AUTHSELECT_PAM_DIR@

install-exec-hook:
	$(MKDIR_P) --mode=a=x,u=rwx $(DESTDIR)$/$(authselect_config_dir)
	$(MKDIR_P) --mode=a=,u=rwx $(DESTDIR)$/$(authselect_vendor_dir)
	$(MKDIR_P) --mode=a=,u=rwx $(DESTDIR)$/$(authselect_custom_dir)
	$(MKDIR_P) --mode=a=rx,u=rwx $(DESTDIR)$/$(authselect_dconf_dir)
	$(MKDIR_P) --mode=a=rx,u=rwx $(DESTDIR)$/$(authselect_dconf_dir)/locks
	$(MKDIR_P) --mode=a=rx,u=rwx $(DESTDIR)$/$(authselect_pam_dir)
