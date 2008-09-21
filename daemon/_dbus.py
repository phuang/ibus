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

class DBusReal(ibus.Object):
    def __init__(self):
        super(DBusReal, self).__init__()
        self.__id_dict = dict()
        self.__conn_dict = dict()
        self.__id = 0

    def register_connection(self, ibusconn):
        if ibusconn in self.__conn_dict:
            return self.__conn_dict[ibusconn]
        self.__id += 1
        id = "%d" % self.__id
        self.__id_dict[id] = ibusconn
        self.__conn_dict[ibusconn] = id
        ibusconn.connect("destroy", self.__ibusconn_destroy_cb, id)
        return id

    def list_names(self):
        return self.__id_dict.keys()

    def list_activatable_names(self):
        return self.__id_dict.keys()

    def name_has_owner(self, name):
        return name in self.__id_dict

    def get_name_owner(self, name):
        if name == dbus.BUS_DAEMON_NAME:
            return dbus.BUS_DAEMON_NAME
        elif name == ibus.IBUS_NAME:
            return ibus.IBUS_NAME

        raise dbus.DBusException(
                "org.freedesktop.DBus.Error.NameHasNoOwner: Could not get owner of name '%s': no such name" % name)

    def start_service_by_name(self, name, flags):
        return 0

    def get_connection_unix_user(self, connection_name):
        return 1

    def add_match(self, rule):
        pass

    def remove_match(self, rule):
        pass

    def __ibusconn_destroy_cb(self, ibusconn, id):
        del self.__id_dict[id]
        del self.__conn_dict[ibusconn]

class DBus(dbus.service.Object):

    method = lambda **args: \
        dbus.service.method(dbus_interface = dbus.BUS_DAEMON_IFACE, \
        **args)

    signal = lambda **args: \
        dbus.service.signal(dbus_interface = dbus.BUS_DAEMON_IFACE, \
        **args)

    __bus = DBusReal()

    def __init__(self, ibusconn):
        super(DBus, self).__init__(ibusconn.get_dbusconn(), dbus.BUS_DAEMON_PATH)
        self.__ibusconn = ibusconn
        self.__name = ""

    @method(out_signature="s")
    def Hello(self):
        if self.__name == None:
            self.__name = DBus.__bus.register_connection(self.__ibusconn)
        return self.__name

    @method(out_signature="as")
    def ListNames(self):
        return DBus.__bus.list_names()

    @method(out_signature="as")
    def ListActivatableNames(self):
        return DBus.__bus.list_activatable_names()

    @method(in_signature="s", out_signature="as")
    def NameHasOwner(self, name):
        return DBus.__bus.name_has_owner(name)

    @method(in_signature="si", out_signature="i")
    def StartServiceByName(self, name, flags):
        return DBus.__bus.start_service_by_name(name, flags)

    @method(in_signature="s", out_signature="s")
    def GetNameOwner(self, name):
        return DBus.__bus.get_name_owner(name)

    @method(in_signature="s", out_signature="i")
    def GetConnectionUnixUser(self, connection_name):
        return DBus.__bus.get_connection_unix_user(connection_name)

    @method(in_signature="s")
    def AddMatch(self, rule):
        return  DBus.__bus.add_match(rule)

    @method(in_signature="s")
    def RemoveMatch(self, rule):
        return  DBus.__bus.remove_match(rule)

    @method(out_signature="s")
    def GetId(self):
        return self.__name

    @signal(signature="sss")
    def NameOwnerChanged(self, name, old_owner, new_owner):
        pass

    @signal(signature="s")
    def NameLost(self, name):
        pass

    @signal(signature="s")
    def NameAcquired(self, name):
        pass
