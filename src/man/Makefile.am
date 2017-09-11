EXTRA_DIST = \
    authselect.8.txt.in \
    authselect-profiles.5.txt.in \
    $(NULL)

expand_prefix = $(SED) -e 's,$${prefix},$(prefix),g'

if HAVE_MANPAGES
CLEANFILES = \
    authselect.8.txt \
    authselect.8.xml \
    authselect-profiles.5.txt \
    authselect-profiles.5.xml \
    $(NULL)

### Man pages
SUFFIXES = \
    .5.txt .5 \
    .8.txt .8 \
    $(NULL)

EXTRA_DIST += \
    authselect.8 \
    authselect-profiles.5 \
    $(NULL)

man_MANS = \
    authselect.8 \
    authselect-profiles.5 \
    $(NULL)

authselect.8.txt:
	$(expand_prefix) < authselect.8.txt.in > authselect.8.txt

authselect-profiles.5.txt:
	$(expand_prefix) < authselect-profiles.5.txt.in > authselect-profiles.5.txt

.8.txt.8: authselect.8.txt
	$(A2X) -D . --doctype manpage --format manpage $<

.5.txt.5: authselect-profiles.5.txt
	$(A2X) -D . --doctype manpage --format manpage $<

clean-local:
	rm -f $(man_MANS)

endif