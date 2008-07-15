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

import weakref
import gobject
import ibus

class Config(ibus.Object):
    __gsignals__ = {
        "value-changed" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_PYOBJECT)),
    }

    def __init__(self, ibusconn, object_path):
        super(Config, self).__init__()
        self._ibusconn = ibusconn
        self._object_path = object_path
        self._config = self._ibusconn.get_object(self._object_path)

        self._ibusconn.connect("destroy", self._ibusconn_destroy_cb)
        self._ibusconn.connect("dbus-signal", self._dbus_signal_cb)

    def get_string(self, key, **kargs):
        self._config.GetString(key, **kargs)

    def get_int(self, key, **kargs):
        self._config.GetInt(key, **kargs)

    def get_bool(self, key, **kargs):
        self._config.GetBool(key, **kargs)

    def set_string(self, key, value, **kargs):
        self._config.SetString(key, value, **kargs)

    def set_int(self, key, value, **kargs):
        self._config.SetInt(key, value, **kargs)

    def set_bool(self, key, value, **kargs):
        self._config.SetBool(key, value, **kargs)

    def destroy(self):
        if self._ibusconn != None:
            self._config.Destroy(**ibus.DEFAULT_ASYNC_HANDLERS)

        self._ibusconn = None
        self._config = None
        ibus.Object.destroy(self)

    # signal callbacks
    def _ibusconn_destroy_cb(self, ibusconn):
        self._ibusconn = None
        self.destroy()

    def _dbus_signal_cb(self, ibusconn, message):
        if message.is_signal(ibus.IBUS_CONFIG_IFACE, "ValueChanged"):
            args = message.get_args_list()
            self.emit("value-changed", args[0], args[1])
        else:
            return False
        return True

gobject.type_register(Config)

class DummyConfig(ibus.Object):
    __gsignals__ = {
        "value-changed" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_PYOBJECT)),
    }

    def get_string(self, key, **kargs):
        kargs["reply_handler"]("")

    def get_int(self, key, **kargs):
        kargs["reply_handler"](0)

    def get_bool(self, key, **kargs):
        kargs["reply_handler"](True)

    def set_string(self, key, value, **kargs):
        kargs["reply_handler"]()

    def set_int(self, key, value, **kargs):
        kargs["reply_handler"]()

    def set_bool(self, key, value, **kargs):
        kargs["reply_handler"]()


gobject.type_register(DummyConfig)
