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

import weakref
import gobject
import ibus
import defaultconfig

class Config(ibus.Object):
    __gsignals__ = {
        "value-changed" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_PYOBJECT)),
    }

    def __init__(self, ibusconn, object_path):
        super(Config, self).__init__()
        self.__ibusconn = ibusconn
        self.__object_path = object_path
        self.__config = self.__ibusconn.get_object(self.__object_path)

        self.__ibusconn.connect("destroy", self.__ibusconn_destroy_cb)
        self.__ibusconn.connect("dbus-signal", self.__dbus_signal_cb)

    def get_value(self, key, **kargs):
        return self.__config.GetValue(key, **kargs)

    def set_value(self, key, value, **kargs):
        return self.__config.GetValue(key, value, **kargs)

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
            self.emit("value-changed", args[0], args[1])
        else:
            return False
        ibusconn.stop_emission("dbus-signal")
        return True

gobject.type_register(Config)

class DefaultConfig(ibus.Object):
    __gsignals__ = {
        "value-changed" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_PYOBJECT)),
    }

    def __init__(self):
        super(DefaultConfig, self).__init__()
        self.__config = defaultconfig.Config()
        self.__handler_id = self.__config.connect("value-changed", self.__value_changed_cb)

    def get_value(self, key, **kargs):
        reply_handler = kargs.get("reply_handler", None)
        error_handler = kargs.get("error_handler", None)
        try:
            value = self.__config.get_value(key)
            if reply_handler:
                reply_handler(value)
            else:
                return value
        except Exception, e:
            if error_handler:
                error_handler(e)
            else:
                raise e

    def set_value(self, key, value, **kargs):
        reply_handler = kargs.get("reply_handler", None)
        error_handler = kargs.get("error_handler", None)
        try:
            self.__config.set_value(key, value)
            if reply_handler:
                reply_handler()
            else:
                return
        except Exception, e:
            if error_handler:
                error_handler(e)
            else:
                raise e

    def __value_changed_cb(self, config, key, value):
        self.emit("value-changed", key, value)

    def do_destroy(self):
        if self.__config:
            self.__config.disconnect(self.__handler_id)
            self.__config.destroy()
            self.__config = None

gobject.type_register(DefaultConfig)
