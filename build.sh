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
$CC $CFLAGS -o ident-allow ../ident_allow.c ../buf.c ../util.c
$CC $CFLAGS -o ident-client ../ident_client.c ../buf.c ../util.c
$CC $CFLAGS -o ident-portmap ../ident_portmap.c ../ident_portmap_"$uname".c ../buf.c ../util.c ../env_write.c
$CC $CFLAGS -o ident-read ../ident_read.c ../buf.c ../util.c
$CC $CFLAGS -o ident-unmap ../ident_unmap.c ../buf.c ../util.c ../env_search.c
$CC $CFLAGS -o ident-usermap ../ident_usermap.c ../buf.c ../util.c ../env_write.c
$CC $CFLAGS -o ident-write ../ident_write.c ../buf.c ../util.c
