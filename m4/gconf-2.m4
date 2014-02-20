dnl AM_GCONF_SOURCE_2
dnl Copied from /usr/share/aclocal/gconf-2.m4 so that ibus build does not
dnl need GConf2-devel

AC_DEFUN([AM_GCONF_SOURCE_2],
[
  if test "x$GCONF_SCHEMA_INSTALL_SOURCE" = "x"; then
    GCONF_SCHEMA_CONFIG_SOURCE=`gconftool-2 --get-default-source`
  else
    GCONF_SCHEMA_CONFIG_SOURCE=$GCONF_SCHEMA_INSTALL_SOURCE
  fi

  AC_ARG_WITH([gconf-source],
	      AC_HELP_STRING([--with-gconf-source=sourceaddress],
			     [Config database for installing schema files.]),
	      [GCONF_SCHEMA_CONFIG_SOURCE="$withval"],)

  AC_SUBST(GCONF_SCHEMA_CONFIG_SOURCE)
  AC_MSG_RESULT([Using config source $GCONF_SCHEMA_CONFIG_SOURCE for schema installation])

  if test "x$GCONF_SCHEMA_FILE_DIR" = "x"; then
    GCONF_SCHEMA_FILE_DIR='$(sysconfdir)/gconf/schemas'
  fi

  AC_ARG_WITH([gconf-schema-file-dir],
	      AC_HELP_STRING([--with-gconf-schema-file-dir=dir],
			     [Directory for installing schema files.]),
	      [GCONF_SCHEMA_FILE_DIR="$withval"],)

  AC_SUBST(GCONF_SCHEMA_FILE_DIR)
  AC_MSG_RESULT([Using $GCONF_SCHEMA_FILE_DIR as install directory for schema files])

  AC_ARG_ENABLE(schemas-install,
  	AC_HELP_STRING([--disable-schemas-install],
		       [Disable the schemas installation]),
     [case ${enableval} in
       yes|no) ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-schemas-install]) ;;
      esac])
  AM_CONDITIONAL([GCONF_SCHEMAS_INSTALL], [test "$enable_schemas_install" != no])
])
