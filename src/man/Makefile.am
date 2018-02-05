expand_prefix =                                                               \
    (                                                                         \
     $(SED) -e 's,$${exec_prefix},$(exec_prefix),g' |                         \
     $(SED) -e 's,$${prefix},$(prefix),g'                                     \
    )

if HAVE_MANPAGES
BUILT_SOURCES = \
    expand_files \
    $(NULL)

man_MANS = \
    authselect.8 \
    authselect-profiles.5 \
    $(NULL)

expand_files:
	for FILE in $(man_MANS) ; do \
		$(expand_prefix) < $$FILE.txt.in > $$FILE.txt ; \
		$(A2X) -D . --doctype manpage --format manpage $$FILE.txt ; \
	done

.PHONY: expand_files

clean-local:
	for FILE in $(man_MANS) ; do \
		$(RM) $$FILE $$FILE.txt ; \
	done
endif