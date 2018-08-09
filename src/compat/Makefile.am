expand_prefix =                                                               \
    (                                                                         \
     $(SED) -e 's,$${exec_prefix},$(exec_prefix),g' |                         \
     $(SED) -e 's,$${prefix},$(prefix),g'                                     \
    )

generated_files = \
    authcompat.py \
    $(NULL)

expand_files:
	for FILE in $(generated_files) ; do \
		$(expand_prefix) < $$FILE.in > $$FILE ; \
	done

.PHONY: expand_files
BUILT_SOURCES = expand_files

CLEANFILES = \
    $(generated_files) \
    $(NULL)

pkgpython_SCRIPTS = \
    authcompat.py \
    $(NULL)

dist_pkgpython_DATA = \
    authcompat_ConfigSnippet.py \
    authcompat_EnvironmentFile.py \
    authcompat_Options.py \
    $(NULL)

snippetsdir = $(pkgpythondir)/snippets
dist_snippets_DATA = \
    snippets/authconfig-krb.conf \
    snippets/authconfig-sssd.conf \
    $(NULL)

# Create symbolic link that will replace authconfig
install-exec-hook:
	$(MKDIR_P) $(DESTDIR)/$(sbindir)
	$(LN_S) -f $(pkgpythondir)/authcompat.py $(DESTDIR)/$(sbindir)/authconfig

uninstall-local:
	$(RM) $(DESTDIR)/$(sbindir)/authconfig
