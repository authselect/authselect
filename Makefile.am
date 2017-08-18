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
    $(NULL)

ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = \
    build/config.rpath \
    rpm/authselect.spec \
    $(NULL)

# Create authselect directories
authselect_config_dir=@AUTHSELECT_CONFIG_DIR@
authselect_profile_dir=@AUTHSELECT_PROFILE_DIR@
authselect_dconf_dir=@AUTHSELECT_DCONF_DIR@

install-exec-hook:
	$(MKDIR_P) --mode=a-rwx,u+rwx,a+x $(DESTDIR)$/$(authselect_config_dir)
	$(MKDIR_P) --mode=a-rwx,u+rwx $(DESTDIR)$/$(authselect_profile_dir)/vendor
	$(MKDIR_P) --mode=a-rwx,u+rwx $(DESTDIR)$/$(authselect_profile_dir)/custom
	$(MKDIR_P) --mode=a=rx,u+rwx $(DESTDIR)$/$(authselect_dconf_dir)
	$(MKDIR_P) --mode=a=rx,u+rwx $(DESTDIR)$/$(authselect_dconf_dir)/locks
