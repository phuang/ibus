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

import dbus.lowlevel
import ibus
import gobject

class Connection(ibus.Object):
    __gsignals__ = {
        "dbus-signal" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_BOOLEAN,
            (gobject.TYPE_PYOBJECT, )
        ),
         "dbus-message" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_BOOLEAN,
            (gobject.TYPE_PYOBJECT, )
        )
   }
    def __init__(self, dbusconn):
        super(Connection, self).__init__()
        self.__dbusconn = dbusconn
        self.__unique_name = ""
        self.__names = set()
        dbusconn.add_message_filter(self.message_filter_cb)

    def message_filter_cb(self, dbusconn, message):
        if message.is_signal(dbus.LOCAL_IFACE, "Disconnected"):
            self.destroy()
            return dbus.lowlevel.HANDLER_RESULT_HANDLED

        if message.get_destination() in set(["org.freedesktop.IBus", "org.freedesktop.DBus"]):
            return dbus.lowlevel.HANDLER_RESULT_NOT_YET_HANDLED

        if message.get_type() == 4: # is signal
            if self.emit("dbus-signal", message):
                return dbus.lowlevel.HANDLER_RESULT_HANDLED
        if self.emit("dbus-message", message):
            return dbus.lowlevel.HANDLER_RESULT_HANDLED
        return dbus.lowlevel.HANDLER_RESULT_NOT_YET_HANDLED

    def get_object(self, path):
        return self.__dbusconn.get_object("no.name", path)

    def emit_dbus_signal(self, name, *args):
        message = dbus.lowlevel.SignalMessage(ibus.IBUS_PATH, ibus.IBUS_IFACE, name)
        message.append(*args)
        self.send_message(message)

    def send_message(self, message):
        self.__dbusconn.send_message(message)
        self.__dbusconn.flush()

    def do_destroy(self):
        self.__dbusconn = None

    def get_dbusconn(self):
        return self.__dbusconn

    def get_unique_name(self):
        return self.__unique_name

    def set_unique_name(self, name):
        assert name
        assert not self.__unique_name
        self.__unique_name = name

    def get_names(self):
        return self.__names

    def add_name(self, name):
        if name not in self.__names:
            self.__names.add(name)

    def remove_name(self, name):
        if name in self.__names:
            self.__names.remove(name)

gobject.type_register(Connection)
