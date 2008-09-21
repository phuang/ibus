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

import dbus
import dbus.service
import ibus

class DBus(dbus.service.Object):

    method = lambda **args: \
        dbus.service.method(dbus_interface = dbus.BUS_DAEMON_IFACE, \
        **args)

    signal = lambda **args: \
        dbus.service.signal(dbus_interface = dbus.BUS_DAEMON_IFACE, \
        **args)
    __id = 0

    def __init__(self, conn):
        super(DBus, self).__init__(conn, dbus.BUS_DAEMON_PATH)

    @method(out_signature="s")
    def Hello(self):
        DBus.__id += 1
        return "%d" % DBus.__id

    @method(out_signature="as")
    def ListNames(self):
        return []

    @method(out_signature="as")
    def ListActivatableNames(self):
        return []

    @method(in_signature="s", out_signature="as")
    def NameHasOwner(self, name):
        return []

    @method(in_signature="si", out_signature="i")
    def StartServiceByName(self, name, flags):
        pass

    @method(in_signature="s", out_signature="s")
    def GetNameOwner(self, name):
        if name == dbus.BUS_DAEMON_NAME:
            return dbus.BUS_DAEMON_NAME
        elif name == ibus.IBUS_NAME:
            return ibus.IBUS_NAME

        raise dbus.DBusException(
                "org.freedesktop.DBus.Error.NameHasNoOwner: Could not get owner of name '%s': no such name" % name)
    @method(in_signature="s", out_signature="i")
    def GetConnectionUnixUser(self, connection_name):
        pass

    @method(in_signature="s")
    def AddMatch(self, rule):
        pass

    @method(in_signature="s")
    def RemoveMatch(self, rule):
        pass

    @method(out_signature="s")
    def GetId(self):
        pass

    @signal(signature="sss")
    def NameOwnerChanged(self, name, old_owner, new_owner):
        pass

    @signal(signature="s")
    def NameLost(self, name):
        pass

    @signal(signature="s")
    def NameAcquired(self, name):
        pass
