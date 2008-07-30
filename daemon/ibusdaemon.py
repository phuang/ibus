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
import dbus
import dbus.server
import dbus.service
import dbus.lowlevel
import dbus.mainloop.glib
import ibus
from bus import IBus

class DBus(dbus.service.Object):

    method = lambda **args: \
        dbus.service.method(dbus_interface = dbus.BUS_DAEMON_IFACE, \
        **args)

    signal = lambda **args: \
        dbus.service.signal(dbus_interface = dbus.BUS_DAEMON_IFACE, \
        **args)

    def __init__(self, *args, **kargs):
        super(DBus, self).__init__(*args, **kargs)

    @method(in_signature = "s", out_signature = "s")
    def GetNameOwner(self, name):
        if name == dbus.BUS_DAEMON_NAME:
            return dbus.BUS_DAEMON_NAME
        elif name == ibus.IBUS_NAME:
            return ibus.IBUS_NAME

        raise dbus.DBusException(
                "org.freedesktop.DBus.Error.NameHasNoOwner: Could not get owner of name '%s': no such name" % name)

    @method(in_signature = "s")
    def AddMatch(self, rule):
        pass

    @signal(signature = "sss")
    def NameOwnerChanged(self, name, old_owner, new_owner):
        pass

class IBusServer(dbus.server.Server):
    def __init__(self, *args, **kargs):
        super(IBusServer, self).__init__()

        self.__ibus = IBus()
        engines = []
        try:
            engines = self.__ibus.config_get_value("auto_enable_engine", None)
            if not engines:
                engines = []
        except:
            import traceback
            traceback.print_exc()
        for e in engines:
            try:
                lang, name = e.split(":")
                self.__ibus.register_start_engine(lang, name, None)
            except:
                import traceback
                traceback.print_exc()

    def connection_added(self, dbusconn):
        self.__ibus.new_connection(dbusconn)
        DBus(dbusconn, dbus.BUS_DAEMON_PATH)

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
