#!/bin/bash
#
#    Authors:
#        Pavel Březina <pbrezina@redhat.com>
#
#    Copyright (C) 2018 Red Hat
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
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
