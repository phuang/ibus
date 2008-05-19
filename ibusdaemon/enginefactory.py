import weakref
import gobject
import ibus
from engine import Engine

class EngineFactory (ibus.Object):
	def __init__ (self, ibusconn, object_path):
		ibus.Object.__init__ (self)
		self._ibusconn = ibusconn
		self._object_path = object_path
		self._factory = self._ibusconn.get_object (self._object_path)

		self._ibusconn.connect ("destroy", self._ibusconn_destroy_cb)
		
		self._ibusconn.connect ("dbus-signal", self._dbus_signal_cb)
		self._engines = weakref.WeakValueDictionary ()

		self._info = None

	def get_object_path (self):
		return self._object_path

	def get_info (self):
		if self._info == None:
			self._info = self._factory.GetInfo ()
		return self._info

	def create_engine (self):
		object_path = self._factory.CreateEngine ()
		engine = Engine (self._ibusconn, object_path)
		self._engines[object_path] = engine
		return engine

	def destroy (self):
		ibus.Object.destroy (self)
		self._ibusconn = None
		self._factory = None

	def _ibusconn_destroy_cb (self, ibusconn):
		self.destroy ()

	def _dbus_signal_cb (self, ibusconn, message):
		object_path = message.get_path ()
		if object_path in self._engines:
			self._engines[object_path].handle_dbus_signal (message)

	# methods for cmp
	def __lt__ (self, other):
		x = self.get_info ()
		y = other.get_info ()
		if x[1] < y[1]: return True
		if x[1] == y[1]: return x[0] < y[0]

	def __gt__ (self, other):
		x = self.get_info ()
		y = other.get_info ()
		if x[1] > y[1]: return True
		if x[1] == y[1]: return x[0] > y[0]

