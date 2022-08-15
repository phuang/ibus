AC_DEFUN([DEFINE_IBUS_LOCALEDIR], [
ibus_save_prefix="$prefix"
ibus_save_datarootdir="$datarootdir"
ibus_save_datadir="$datadir"
ibus_save_localedir="$localedir"
test "x$prefix" = xNONE && prefix=$ac_default_prefix
datarootdir=`eval echo "$datarootdir"`
datadir=`eval echo "$datadir"`
test "x$localedir" = xNONE && localedir="${datadir}/locale"
ibus_localedir=`eval echo "$localedir"`
localedir="$ibus_save_localedir"
datadir="$ibus_save_datadir"
datarootdir="$ibus_save_datarootdir"
prefix="$ibus_save_prefix"
])
