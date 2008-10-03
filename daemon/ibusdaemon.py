# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA

import os
import sys
import getopt
import getpass
import gobject
import dbus.server
import dbus.mainloop.glib
import ibus
from _dbus import DBus
from bus import IBus
from connection import Connection


class IBusServer(dbus.server.Server):
    def __init__(self, *args, **kargs):
        super(IBusServer, self).__init__()

        self.__ibus = IBus()
        self.__launch_auto_load_engines()

    def __launch_auto_load_engines(self):
        engines = []
        try:
            engines = self.__ibus.config_get_value("general", "preload_engines", None)
            if not engines:
                engines = []
        except:
            pass
        for e in engines:
            try:
                lang, name = e.split(":")
                self.__ibus.register_start_engine(lang, name, None)
            except:
                pass

    def connection_added(self, dbusconn):
        conn = Connection(dbusconn)
        self.__ibus.new_connection(conn)
        DBus(conn)

    def connection_removed(self, dbusconn):
        # do nothing.
        pass

def launch_ibus():
    dbus.mainloop.glib.DBusGMainLoop(set_as_default = True)
    loop = gobject.MainLoop()
    try:
        os.mkdir("/tmp/ibus-%s" % getpass.getuser())
    except:
        pass
    bus = IBusServer(ibus.IBUS_ADDR)
    try:
        loop.run()
    except KeyboardInterrupt, e:
        print "daemon exits"
        sys.exit()


def print_help(out, v = 0):
    print >> out, "-h, --help             show this message."
    print >> out, "-d, --daemonize        daemonize ibus"
    sys.exit(v)

def main():
    daemonize = False
    shortopt = "hd"
    longopt = ["help", "daemonize"]
    try:
        opts, args = getopt.getopt(sys.argv[1:], shortopt, longopt)
    except getopt.GetoptError, err:
        print_help(sys.stderr, 1)

    for o, a in opts:
        if o in ("-h", "--help"):
            print_help(sys.stdout)
        elif o in ("-d", "--daemonize"):
            daemonize = True
        else:
            print >> sys.stderr, "Unknown argument: %s" % o
            print_help(sys.stderr, 1)

    if daemonize:
        if os.fork():
            sys.exit()

    launch_ibus()


if __name__ == "__main__":
    main()
