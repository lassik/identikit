#!/bin/sh
set -eu
uname="$(uname | tr A-Z a-z)"
builddir="build-$uname-$(uname -m)"
[ -d "$builddir" ] || mkdir -m 0700 "$builddir"
find "$builddir" -depth -mindepth 1 | xargs rm -rf --
cd "$builddir"
echo "Entering directory '$PWD'"
export CC="${CC:-clang}"
export CFLAGS="${CFLAGS:--Wall -Wextra -g}"
set -x
$CC $CFLAGS -o ident-client ../ident_client.c ../buf.c
$CC $CFLAGS -o ident-read ../ident_read.c ../ident_read_"$uname".c ../buf.c
$CC $CFLAGS -o ident-user ../ident_user.c ../buf.c
$CC $CFLAGS -o ident-usergen ../ident_usergen.c ../buf.c
$CC $CFLAGS -o ident-write ../ident_write.c ../buf.c
