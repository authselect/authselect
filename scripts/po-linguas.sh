#!/bin/bash
#
# Search for available translations under po directory and create
# po/LINGUAS file.
#
# Usage: po-linguas.sh SOURCE_DIRECTORY
#

# Create po/LINGUAS

SRCDIR="$1"
DESTFILE="$1/po/LINGUAS"

echo "Creating $DESTFILE"
find "$SRCDIR/po" -type f -name '*.po' -printf '%d\t%f\n' | \
    sort -nk1 | cut -f2- | sed 's/\.po$//' > "$DESTFILE"
