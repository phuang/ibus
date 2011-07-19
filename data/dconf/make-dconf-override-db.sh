#!/bin/bash

set -e

export TMPDIR=$(mktemp -d --tmpdir="$PWD")
export XDG_CONFIG_HOME="$TMPDIR/config"
export XDG_CACHE_HOME="$TMPDIR/cache"
export GSETTINGS_SCHEMA_DIR="$TMPDIR/schemas"
mkdir -p $XDG_CONFIG_HOME $XDG_CACHE_HOME $GSETTINGS_SCHEMA_DIR

eval `dbus-launch --sh-syntax`

trap 'rm -rf $TMPDIR; kill $DBUS_SESSION_BUS_PID' ERR

# in case that schema is not installed on the system
glib-compile-schemas --targetdir "$GSETTINGS_SCHEMA_DIR" "$PWD"

gsettings list-recursively org.freedesktop.ibus.general | \
while read schema key val; do
    gsettings set "$schema" "$key" "$val"
done

gsettings list-recursively org.freedesktop.ibus.panel | \
while read schema key val; do
    gsettings set "$schema" "$key" "$val"
done

mv $XDG_CONFIG_HOME/dconf/user "$1"
rm -rf $TMPDIR

kill $DBUS_SESSION_BUS_PID
