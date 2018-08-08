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
# Zanata hooks to automate push and pull into and from Zanata.
# - push hooks make sure that manual pages are translated into pot files
# - pull hooks populates po/LINGUAS with available languages
#
# Usage: zanata-hooks.sh [push-before|push-after|pull-before|pull-after]
#

zanata_push_before()
{
    # Update C pot file
    make -C "$PWD/po" update-po &> /dev/null
    if [ $? -ne 0 ]; then
        echo "Unable to run: make -C \"$PWD/po\" update-po"
        echo "Make sure that ./configure was run and the project is configured"
        exit 1
    fi

    # Update man pages pot files
    DIR="$PWD/src/man"
    for file in $DIR/*.adoc; do
        name=`basename $file`
        po4a-gettextize -f asciidoc -m $file -p "$DIR/po/$name.pot" > /dev/null
        if [ $? -ne 0 ]; then
            exit 1
        fi
    done
}

zanata_pull_after()
{
    # Update LINGUAS file
    $PWD/scripts/po-linguas.sh $PWD
}

case $1 in
push-before)
  zanata_push_before
  ;;
push-after)
  ;;
pull-before)
  ;;
pull-after)
  zanata_pull_after
  ;;
*)
  echo "Unknown parameter $1"
  exit 1
  ;;
esac

exit 0
