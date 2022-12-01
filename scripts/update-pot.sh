#!/bin/bash
#
#    Authors:
#        Pavel Březina <pbrezina@redhat.com>
#
#    Copyright (C) 2020 Red Hat
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
# Update pot files.
#
# Usage: update-pot.sh
#

if [ ! -f "rpm/authselect.spec.in" ]; then
    echo "Make sure you are in authselect root directory."
    exit 1
fi

git diff-index --quiet HEAD -- &> /dev/null
if [ $? -ne 0 ]; then
    echo "Please commit all changes first."
    exit 1
fi

# Update C pot file
make distclean &> /dev/null
./configure

make -C "po" update-po &> /dev/null
if [ $? -ne 0 ]; then
    echo 'Unable to run: make -C "po" update-po'
    exit 1
fi

# Update man pages pot files
DIR="./src/man"
for file in $DIR/*.adoc; do
    name=`basename $file`
    po4a-updatepo -f asciidoc -m $file -p "$DIR/po/$name.pot" > /dev/null
    if [ $? -ne 0 ]; then
        exit 1
    fi
done

# Exclude changes in po files
git checkout po/*.po

# Commit changes
git commit -a -m "pot: update pot files"

# Cleanup
make distclean &> /dev/null
