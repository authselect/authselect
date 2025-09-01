SUBDIRS= \
    po \
    profiles \
    src/common \
    src/lib \
    src/cli \
    src/tests \
    systemd \
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
    scripts/auto \
    scripts/commands/install-build-deps.sh \
    scripts/commands/make-srpm.sh \
    scripts/release-version.sh \
    scripts/po-linguas.sh \
    $(NULL)

# Create authselect directories
authselect_config_dir=@AUTHSELECT_CONFIG_DIR@
authselect_vendor_dir=@AUTHSELECT_VENDOR_DIR@
authselect_custom_dir=@AUTHSELECT_CUSTOM_DIR@
authselect_dconf_dir=@AUTHSELECT_DCONF_DIR@
authselect_pam_dir=@AUTHSELECT_PAM_DIR@
authselect_backup_dir=@AUTHSELECT_BACKUP_DIR@

install-exec-hook:
	$(MKDIR_P) $(DESTDIR)$/$(authselect_config_dir)
	$(MKDIR_P) $(DESTDIR)$/$(authselect_vendor_dir)
	$(MKDIR_P) $(DESTDIR)$/$(authselect_custom_dir)
	$(MKDIR_P) $(DESTDIR)$/$(authselect_dconf_dir)
	$(MKDIR_P) $(DESTDIR)$/$(authselect_dconf_dir)/locks
	$(MKDIR_P) $(DESTDIR)$/$(authselect_pam_dir)
	$(MKDIR_P) $(DESTDIR)$/$(authselect_backup_dir)

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
	CFLAGS="$(CFLAGS) -fPIC" rpmbuild --define "_topdir $(RPMBUILD)" -ba SPECS/authselect.spec

srpm: rpmprep
	cd $(RPMBUILD); \
	rpmbuild --define "_topdir $(RPMBUILD)" -bs SPECS/authselect.spec
