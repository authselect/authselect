SUBDIRS= \
    po \
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
EXTRA_DIST = build/config.rpath
