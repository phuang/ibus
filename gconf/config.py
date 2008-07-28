import gconf
import ibus
from ibus import interface

class Config(ibus.Object):
    def __init__ (self, conn, path):
		super(Config, self).__init__()
        self.__proxy = ConfigProxy(self, conn, path)
        self.__client = gconf.Client()
        self.__client.connect("value-changed", self.__value_changed_cb)
        self.__client.add_dir("/", gconf.CLIENT_PRELOAD_NONE)

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

    def __value_changed_cb(self, gconf, key, value):
        value = self.__client.get_value(key)
        print key, type(value), value
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
