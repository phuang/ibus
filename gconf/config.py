import gconf
import ibus
from ibus import interface

class Config (ibus.Object):
	def __init__ (self, proxy):
		ibus.Object.__init__ (self)
		self._proxy = proxy
		self._client = gconf.Client ()
		self._client.connect ("value-changed", self._value_changed_cb)
		self._client.add_dir ("/", gconf.CLIENT_PRELOAD_NONE)

	def get_string (self, key):
		pass
	def get_int (self, key):
		pass
	def get_bool (self, key):
		pass
	
	def set_string (self, key, value):
		pass
	def set_int (self, key, value):
		pass
	def set_bool (self, key, value):
		pass

	def _value_changed_cb (self, gconf, key, value):
		value = self._client.get_value (key)
		print key, type (value), value
		self._proxy.ValueChanged (key, value)

class ConfigProxy (interface.IConfig):
	def __init__ (self, dbusconn, object_path, _ibus):
		interface.IConfig.__init__ (self, dbusconn, object_path)
		self._dbusconn = dbusconn
		self._config = Config (self)

	def GetString (self, key):
		return self._config.get_string (key)
	def GetInt (self, key):
		return self._config.get_int (key)
	def GetBool (self, key):
		return self._config.get_bool (key)

	def SetString (self, key, value):
		self._config.set_string (key, value)
	def SetInt (self, key, value):
		self._config.set_int (key, value)
	def SetBool (self, key, value):
		self._config.set_bool (key, value)
