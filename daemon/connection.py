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
        )
    }
    def __init__(self, dbusconn):
        super(Connection, self).__init__()
        self.__dbusconn = dbusconn
        self.__config_watch = set()
        dbusconn.add_message_filter(self.message_filter_cb)

    def message_filter_cb(self, dbusconn, message):
        if message.is_signal(dbus.LOCAL_IFACE, "Disconnected"):
            self.destroy()
            return dbus.lowlevel.HANDLER_RESULT_HANDLED

        if message.get_type() == 4: # is signal
            if self.dispatch_dbus_signal(message):
                return dbus.lowlevel.HANDLER_RESULT_HANDLED

        return dbus.lowlevel.HANDLER_RESULT_NOT_YET_HANDLED

    def get_object(self, path):
        return self.__dbusconn.get_object("no.name", path)

    def emit_dbus_signal(self, name, *args):
        message = dbus.lowlevel.SignalMessage(ibus.IBUS_PATH, ibus.IBUS_IFACE, name)
        message.append(*args)
        self.__dbusconn.send_message(message)
        self.__dbusconn.flush()

    def do_destroy(self):
        self.__dbusconn = None

    def dispatch_dbus_signal(self, message):
        return self.emit("dbus-signal", message)

    def config_add_watch(self, key):
        if key in self.__config_watch:
            return False
        self.__config_watch.add(key)
        return True

    def config_remove_watch(self, key):
        if key not in self.__config_watch:
            return False
        self.__config_watch.remove(key)
        return True

    def config_get_watch(self):
        return self.__config_watch.copy()

    def get_dbusconn(self):
        return self.__dbusconn

gobject.type_register(Connection)
