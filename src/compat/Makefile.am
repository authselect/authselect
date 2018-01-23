expand_prefix =                                                               \
    (                                                                         \
     $(SED) -e 's,$${exec_prefix},$(exec_prefix),g' |                         \
     $(SED) -e 's,$${prefix},$(prefix),g'                                     \
    )

generated_files = \
    authcompat_paths \
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

dist_pkgpython_SCRIPTS = \
    authcompat.py \
    $(NULL)

dist_pkgpython_DATA = \
    authcompat_ConfigSnippet.py \
    authcompat_EnvironmentFile.py \
    authcompat_Options.py \
    $(NULL)

pkgpython_DATA = \
    $(generated_files) \
    $(NULL)

# Create symbolic link that will replace authconfig
install-exec-hook:
	$(MKDIR_P) $(DESTDIR)/$(sbindir)
	$(LN_S) -f $(pkgpythondir)/authcompat.py $(DESTDIR)/$(sbindir)/authconfig

uninstall-local:
	$(RM) $(DESTDIR)/$(sbindir)/authconfig
