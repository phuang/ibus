#!/bin/sh
# -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*-
# vim:set noet ts=4:
#
# ibus-anthy - The Anthy engine for IBus
#
# Copyright (c) 2018-2019 Takao Fujiwara <takao.fujiwara1@gmail.com>
# Copyright (c) 2018 Red Hat, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# This test runs /usr/bin/ibus-daemon after install ibus
#
# # init 3
# Login as root
# # /root/ibus/tests/test-console.sh --tests ibus-compose \
#   --builddir /root/ibus/src/tests --srcdir /root/ibus/src/tests

PROGNAME=`basename $0`
VERSION=0.1
DISPLAY=:99.0
BUILDDIR="."
SRCDIR="."
TEST_LOG=test-suite.log
HAVE_GRAPHICS=1
DESKTOP_COMMAND="gnome-session"
PID_XORG=0
PID_GNOME_SESSION=0
TESTS=""
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

usage()
{
    echo -e \
"This test runs /usr/bin/ibus-daemon after install ibus\n"                     \
"$PROGNAME [OPTIONS…]\n"                                                       \
"\n"                                                                           \
"OPTIONS:\n"                                                                   \
"-h, --help                       This help\n"                                 \
"-v, --version                    Show version\n"                              \
"-b, --builddir=BUILDDIR          Set the BUILDDIR\n"                          \
"-s, --srcdir=SOURCEDIR           Set the SOURCEDIR\n"                         \
"-c, --no-graphics                Use Xvfb instead of Xorg\n"                  \
"-d, --desktop=DESKTOP            Run DESTKTOP. The default is gnome-session\n" \
"-t, --tests=\"TESTS...\"           Run TESTS programs which is separated by space\n" \
""
}

parse_args()
{
    # This is GNU getopt. "sudo port getopt" in BSD?
    ARGS=`getopt -o hvb:s:cd:t: --long help,version,builddir:,srcdir:,no-graphics,desktop:,tests:\
        -- "$@"`;
    eval set -- "$ARGS"
    while [ 1 ] ; do
        case "$1" in
        -h | --help )        usage; exit 0;;
        -v | --version )     echo -e "$VERSION"; exit 0;;
        -b | --builddir )    BUILDDIR="$2"; shift 2;;
        -s | --srcdir )      SRCDIR="$2"; shift 2;;
        -c | --no-graphics ) HAVE_GRAPHICS=0; shift;;
        -d | --desktop )     DESKTOP_COMMAND="$2"; shift 2;;
        -t | --tests )       TESTS="$2"; shift 2;;
        -- )                 shift; break;;
        * )                  usage; exit 1;;
        esac
    done
}

init_desktop()
{
    if test x$FORCE_TEST != x ; then
        RUN_ARGS="$RUN_ARGS --force"
    fi

    if test ! -f $HOME/.config/gnome-initial-setup-done ; then
        if test ! -f /var/lib/AccountsService/users/$USER ; then
            mkdir -p /var/lib/AccountsService/users
            cat >> /var/lib/AccountsService/users/$USER << _EOF
[User]
Language=ja_JP.UTF-8
XSession=gnome
SystemAccount=false
_EOF
        fi
        mkdir -p $HOME/.config
        touch $HOME/.config/gnome-initial-setup-done
    fi

    # Prevent from launching a XDG dialog
    XDG_LOCALE_FILE="$HOME/.config/user-dirs.locale"
    if test -f $XDG_LOCALE_FILE ; then
        XDG_LANG_ORIG=`cat $XDG_LOCALE_FILE`
        XDG_LANG_NEW=`echo $LANG | sed -e 's/\(.*\)\..*/\1/'`
        if [ "$XDG_LANG_ORIG" != "$XDG_LANG_NEW" ] ; then
            echo "Overriding XDG locale $XDG_LANG_ORIG with $XDG_LANG_NEW"
            echo "$XDG_LANG_NEW" > $XDG_LOCALE_FILE
        fi
    fi
}

run_dbus_daemon()
{
    a=`ps -ef | grep dbus-daemon | grep "\-\-system" | grep -v session | grep -v grep`
    if test x"$a" = x ; then
        eval `dbus-launch --sh-syntax`
    fi
    SUSER=`echo "$USER" | cut -c 1-7`
    a=`ps -ef | grep dbus-daemon | grep "$SUSER" | grep -v gdm | grep session | grep -v grep`
    if test x"$a" = x ; then
        systemctl --user start dbus
        export DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$UID/bus
    fi
    systemctl --user status dbus | col -b
    ps -ef | grep dbus-daemon | grep "$SUSER" | grep -v gdm | egrep 'session|system' | grep -v grep
    systemctl --user show-environment | col -b
}

run_desktop()
{
    if test $HAVE_GRAPHICS -eq 1 ; then
        /usr/libexec/Xorg.wrap -noreset +extension GLX +extension RANDR +extension RENDER -logfile ./xorg.log -config ./xorg.conf -configdir . $DISPLAY &
    else
        /usr/bin/Xvfb $DISPLAY -noreset +extension GLX +extension RANDR +extension RENDER -screen 0 1280x1024x24 &
    fi
    PID_XORG=$!
    sleep 1
    export DISPLAY=$DISPLAY
    $DESKTOP_COMMAND &
    PID_GNOME_SESSION=$!
    sleep 30
    if test "$DESKTOP_COMMAND" != "gnome-session" ; then
        ibus-daemon --daemonize --verbose
        sleep 1
    fi
}

count_case_result()
{
    retval=$1
    pass=$2
    fail=$3

    if test $retval -eq  0 ; then
        pass=`expr $pass + 1`
    else
        fail=`expr $fail + 1`
    fi
    echo $pass $fail
}

echo_case_result()
{
    retval=$1
    tst=$2
    log=$3
    subtst=${4:-''}

    if test $retval -eq  0 ; then
        echo -e "${GREEN}PASS${NC}: $tst $subtst"
    else
        echo -e "${RED}FAIL${NC}: $tst $subtst"
        echo "FAIL: $tst $subtst" >> $TEST_LOG
        echo "======================" >> $TEST_LOG
        echo "" >> $TEST_LOG
        cat "$log" >> $TEST_LOG
        echo "" >> $TEST_LOG
    fi
}

run_test_suite()
{
    cd `dirname $0`
    pass=0
    fail=0

    export GTK_IM_MODULE=ibus
    export IBUS_COMPOSE_CACHE_DIR=$PWD
    if test -f $TEST_LOG ; then
        rm $TEST_LOG
    fi
    for tst in $TESTS; do
        ENVS=
        if test -f $SRCDIR/${tst}.env ; then
            ENVS="`cat $SRCDIR/${tst}.env`"
        fi
        if test x"$ENVS" = x ; then
            $BUILDDIR/$tst $SRCDIR >&${tst}.log
            retval=$?
            read pass fail << EOF
            `count_case_result $retval $pass $fail`
EOF
            echo_case_result $retval $tst ${tst}.log
        else
            LANG_backup=$LANG
            i=1
            for e in $ENVS; do
                first=`echo "$e" | cut -c1-1`
                if test x"$first" = x"#" ; then
                    continue
                fi
                export $e
                $BUILDDIR/$tst $SRCDIR >&${tst}.${i}.log
                retval=$?
                read pass fail << EOF
                `count_case_result $retval $pass $fail`
EOF
                echo_case_result $retval $tst ${tst}.${i}.log $e
                i=`expr $i + 1`
            done
            export LANG=$LANG_backup
        fi
    done
    echo ""
    echo -e "# ${GREEN}PASS${NC}: $pass"
    echo -e "# ${RED}FAIL${NC}: $fail"
    if test -f ${TEST_LOG} ; then
        echo ""
        echo -e "${RED}See ${TEST_LOG}$NC"
    fi
}

finit()
{
    if test "$DESKTOP_COMMAND" != "gnome-session" ; then
        ibus exit
    fi
    kill $PID_GNOME_SESSION $PID_XORG
}

main()
{
    parse_args $@
    init_desktop
    run_dbus_daemon
    run_desktop
    run_test_suite
    finit
}

main $@
