#!/bin/bash
#
# Print current release version. If argument RELEASE is not available then
# current date is taken.
#
# Usage: release-version.sh [RELEASE]
#

if [ -z "$1" ]; then
    echo `date +%Y%m%d.%H%M`
    exit 0
fi

echo $1

exit 0
