#!/bin/sh
set -eu
cd "$(dirname "$0")"
uname="$(uname | tr A-Z a-z)"
builddir="build-$uname-$(uname -m)"
installdir=/usr/local/libexec/identikit
set -x
sudo mkdir -p "$installdir"
sudo install "$builddir/"ident-client  "$installdir/"
sudo install "$builddir/"ident-port    "$installdir/"
sudo install "$builddir/"ident-portmap "$installdir/"
sudo install "$builddir/"ident-read    "$installdir/"
sudo install "$builddir/"ident-user    "$installdir/"
sudo install "$builddir/"ident-usermap "$installdir/"
sudo install "$builddir/"ident-write   "$installdir/"
