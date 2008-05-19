import weakref
import gobject
import ibus

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

