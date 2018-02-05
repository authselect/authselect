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
    release-version.sh \
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

# Build RPMs
RPMBUILD ?= $(PWD)/rpmbuild

rpmroot:
	$(MKDIR_P) $(RPMBUILD)/BUILD
	$(MKDIR_P) $(RPMBUILD)/RPMS
	$(MKDIR_P) $(RPMBUILD)/SOURCES
	$(MKDIR_P) $(RPMBUILD)/SPECS
	$(MKDIR_P) $(RPMBUILD)/SRPMS

rpmprep: dist-gzip rpmroot
	cp $(builddir)/rpm/authselect.spec $(RPMBUILD)/SPECS
	cp $(distdir).tar.gz $(RPMBUILD)/SOURCES

rpms: rpmprep
	cd $(RPMBUILD); \
	rpmbuild --define "_topdir $(RPMBUILD)" -ba SPECS/authselect.spec

srpm: rpmprep
	cd $(RPMBUILD); \
	rpmbuild --define "_topdir $(RPMBUILD)" -bs SPECS/authselect.spec
