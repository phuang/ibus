# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright(c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or(at your option) any later version.
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

import gconf
import ibus
from ibus import interface

GCONF_IBUS_PATH = "/desktop/ibus"

class Config(ibus.Object):
    def __init__ (self, conn, path):
        super(Config, self).__init__()
        self.__proxy = ConfigProxy(self, conn, path)
        self.__client = gconf.Client()
        self.__handler_id = self.__client.connect("value-changed", self.__value_changed_cb)
        self.__client.add_dir(GCONF_IBUS_PATH, gconf.CLIENT_PRELOAD_NONE)

    def get_string(self, key):
        pass
    def get_int(self, key):
        pass
    def get_bool(self, key):
        pass

    def set_string(self, key, value):
        pass
    def set_int(self, key, value):
        pass
    def set_bool(self, key, value):
        pass

    def do_destroy(self):
        self.__proxy = None
        self.__client.disconnect(self.__handler_id)
        self.__client = None

    def __to_py_value(self, value):
        if value.type == gconf.VALUE_STRING:
            return value.get_string()
        if value.type == gconf.VALUE_INT:
            return value.get_int()
        if value.type == gconf.VALUE_FLOAT:
            return value.get_float()
        if value.type == gconf.VALUE_BOOL:
            return value.get_bool()
        if value.type == gconf.VALUE_PAIR:
            return (self.__to_py_value(value.get_car()), self.__to_py_value(value.get_cdr()))
        if value.type == gconf.VALUE_LIST:
            return map(self.__to_py_value, value.get_list())

    def __to_gconf_value(self, value):
        if isinstance(value, str):
            ret = gconf.Value(gconf.VALUE_STRING)
            ret.set_string(value)
        elif isinstance(value, int):
            ret = gconf.Value(gconf.VALUE_INT)
            ret.set_int(value)
        elif isinstance(value, float):
            ret = gconf.Value(gconf.VALUE_FLOAT)
            ret.set_float(value)
        elif isinstance(value, bool):
            ret = gconf.Value(gconf.VALUE_BOOL)
            ret.set_bool(value)
        elif isinstance(value, tuple):
            if len(value) != 2:
                raise ibus.IBusException("Pair must have two value")
            ret = gconf.Value(gconf.VALUE_PAIR)
            ret.set_car(self.__to_gconf_value(value[0]))
            ret.set_crd(self.__to_gconf_value(value[1]))
        elif isinstance(value, list):
            ret = gconf.Value(gconf.VALUE_LIST)
            if len(value) > 0:
                value = map(self.__to_gconf_value, value)
                _type = value[0].type
                if any(map(lambda x: x.type != _type, value)):
                    raise ibus.IBusException("Items of a list must be in same type")
                ret.set_list_type(_type)
                ret.set_list(value)
        return ret

    def __value_changed_cb(self, gconf, key, value):
        value = self.__client.get(key)
        value = self.__to_py_value(value)

        print key, type(value), value
        print key, type(value), self.__to_gconf_value(value)
        self.__proxy.ValueChanged(key, value)

class ConfigProxy(interface.IConfig):
    def __init__ (self, config, conn, object_path):
        super(ConfigProxy, self).__init__(conn, object_path)
        self.__config = config

    def GetString(self, key):
        return self.__config.get_string(key)
    def GetInt(self, key):
        return self.__config.get_int(key)
    def GetBool(self, key):
        return self.__config.get_bool(key)

    def SetString(self, key, value):
        self.__config.set_string(key, value)
    def SetInt(self, key, value):
        self.__config.set_int(key, value)
    def SetBool(self, key, value):
        self.__config.set_bool(key, value)

    def Destroy(self):
        self.remove_from_connection()
        self.__config.destroy()
        self.__config = None
