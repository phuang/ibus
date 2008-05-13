import weakref
import gobject
import ibus

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


class Engine (ibus.Object):
	__gsignals__ = {
		"commit-string" : (
			gobject.SIGNAL_RUN_FIRST, 
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, )),
		"preedit-changed" : (
			gobject.SIGNAL_RUN_FIRST, 
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_PYOBJECT, gobject.TYPE_UINT))
	}

	def __init__ (self, ibusconn, object_path):
		ibus.Object.__init__ (self)
		self._ibusconn = ibusconn
		self._object_path = object_path
		self._engine = ibusconn.get_object (self._object_path)
		self._ibusconn.connect ("destroy", self._ibusconn_destroy_cb)

	def handle_dbus_signal (self, message):
		if message.is_signal (ibus.IBUS_ENGINE_IFACE, "CommitString"):
			args = message.get_args_list ()
			self.emit ("commit-string", args[0])
			return True
		elif message.is_signal (ibus.IBUS_ENGINE_IFACE, "PreeditChanged"):
			args = message.get_args_list ()
			self.emit ("preedit-changed", args[0], args[1], args[2])
			return True
		else:
			return False

	def focus_in (self):
		self._engine.FocusIn (**ibus.DEFAULT_ASYNC_HANDLERS)

	def focus_out (self):
		self._engine.FocusOut (**ibus.DEFAULT_ASYNC_HANDLERS)

	def reset (self):
		self._engine.Reset (**ibus.DEFAULT_ASYNC_HANDLERS)

	def process_key_event (self, keyval, is_press, state, reply_cb, error_cb):
		self._engine.ProcessKeyEvent (keyval, is_press, state, 
									reply_handler = reply_cb,
									error_handler = error_cb)
	
	def set_cursor_location (self, x, y, w, h):
		self._engine.SetCursorLocation (x, y, w, h)

	def set_enable (self, enable):
		self._engine.SetEnable (enable)

	def destroy (self):
		ibus.Object.destroy (self)
		if self._engine:
			self._engine.Destroy ()
			self._engine = None
		self._ibusconn = None

	def _ibusconn_destroy_cb (self, ibusconn):
		self._engine = None
		self.destroy ()

gobject.type_register (Engine)

