
EXTRA_DIST = \
	authselect.8.txt \
	$(NULL)


if HAVE_MANPAGES
### Man pages
SUFFIXES = .8.txt .8

EXTRA_DIST += \
	authselect.8 \
	$(NULL)

man_MANS = \
	authselect.8 \
	$(NULL)

.8.txt.8:
	$(A2X) -D . --doctype manpage --format manpage $<

clean-local:
	rm -f $(man_MANS)
endif