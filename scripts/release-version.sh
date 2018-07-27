#!/bin/bash

if [ -z "$1" ]; then
    echo `date +%Y%m%d.%H%M`
    exit 0
fi

echo $1

exit 0
