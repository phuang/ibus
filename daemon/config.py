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

__all__ = (
    "Config",
    "DefaultConfig"
)

import gobject
import ibus

class Config(ibus.Object):
    __gsignals__ = {
        "value-changed" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_STRING, gobject.TYPE_PYOBJECT)),
    }

    def __init__(self, ibusconn):
        super(Config, self).__init__()
        self.__ibusconn = ibusconn
        self.__config = self.__ibusconn.get_object(ibus.IBUS_CONFIG_PATH)

        self.__ibusconn.connect("destroy", self.__ibusconn_destroy_cb)
        self.__ibusconn.connect("dbus-signal", self.__dbus_signal_cb)

    def get_value(self, section, name, **kargs):
        return self.__config.GetValue(section, name, **kargs)

    def set_value(self, section, name, value, **kargs):
        return self.__config.SetValue(section, name, value, **kargs)

    def destroy(self):
        if self.__ibusconn != None:
            self.__config.Destroy(**ibus.DEFAULT_ASYNC_HANDLERS)

        self.__ibusconn = None
        self.__config = None
        ibus.Object.destroy(self)

    # signal callbacks
    def __ibusconn_destroy_cb(self, ibusconn):
        self.__ibusconn = None
        self.destroy()

    def __dbus_signal_cb(self, ibusconn, message):
        if message.is_signal(ibus.IBUS_CONFIG_IFACE, "ValueChanged"):
            args = message.get_args_list()
            self.emit("value-changed", args[0], args[1], args[2])
        return False

gobject.type_register(Config)

class DummyConfig(ibus.Object):
    __gsignals__ = {
        "value-changed" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_STRING, gobject.TYPE_PYOBJECT)),
    }

    def __init__(self):
        super(DummyConfig, self).__init__()
        self.__values = dict()

    def get_value(self, section, name, **kargs):
        value = self.__values.get((section, name), None)
        if value == None:
            raise ibus.IBusException("Can not get config section=%s name=%s" % (section, name))
        return value

    def set_value(self, section, name, value, **kargs):
        old_value = self.__values.get((section, name), None)
        if value == old_value:
            return
        if value == None:
            del self.__values[(section, name)]
        else:
            self.__values[(section, name)] = value
        self.emit("value-changed", section, name, value)

gobject.type_register(DummyConfig)
