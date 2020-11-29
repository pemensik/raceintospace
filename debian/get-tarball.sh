#!/bin/bash

set -eu
repo=https://github.com/raceintospace/raceintospace.git

if [ $# = 0 ]; then
    echo "Available tags:"
    git ls-remote $repo \
        | awk '{ print $2 }' | egrep 'refs/tags/[0-9]+' \
        | awk 'FS="/" { print $3 }' | xargs echo
    exit 0
fi

version=$1

tmp=$(mktemp -d)
name="raceintospace-${version}.tar.xz"

(
    cd $tmp
    git clone ${repo} raceintospace
    cd raceintospace
    if [ "$version" = "1.1" ]; 
    then
     git reset --hard 014a37c3649790559bf376ba0595e0416dc29c91
     else
    git reset --hard $version
    fi
    rm -rf ./.git ./.gitignore ./.git-hooks/ ./src/game/platform_macosx
    tar cfJ ../${name} .
)

cp ${tmp}/${name} ../

rm -rf ${tmp}

echo "The archive saved as ../${name}"
