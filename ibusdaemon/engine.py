import weakref
import gobject
import ibus

class Engine (ibus.Object):
	__gsignals__ = {
		"commit-string" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, )),
		"forward-key-event" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_UINT, gobject.TYPE_BOOLEAN, gobject.TYPE_UINT )),
		"update-preedit" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_PYOBJECT, gobject.TYPE_INT, gobject.TYPE_BOOLEAN)),
		"update-aux-string" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_PYOBJECT, gobject.TYPE_BOOLEAN)),
		"update-lookup-table" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_PYOBJECT, gobject.TYPE_BOOLEAN)),
		"register-properties" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_PYOBJECT, )),
		"update-property" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_PYOBJECT, )),
	}

	def __init__ (self, factory, ibusconn, object_path):
		ibus.Object.__init__ (self)
		self._factory = factory
		self._ibusconn = ibusconn
		self._object_path = object_path
		self._engine = ibusconn.get_object (self._object_path)
		self._lookup_table = ibus.LookupTable ()
		self._ibusconn.connect ("destroy", self._ibusconn_destroy_cb)

	def get_factory (self):
		return self._factory

	def handle_dbus_signal (self, message):
		if message.is_signal (ibus.IBUS_ENGINE_IFACE, "CommitString"):
			args = message.get_args_list ()
			self.emit ("commit-string", args[0])
		elif message.is_signal (ibus.IBUS_ENGINE_IFACE, "ForwardKeyEvent"):
			args = message.get_args_list ()
			self.emit ("forward-key-event", args[0], bool (arg[1]), arg[2])
		elif message.is_signal (ibus.IBUS_ENGINE_IFACE, "UpdatePreedit"):
			args = message.get_args_list ()
			self.emit ("update-preedit", args[0], args[1], args[2], args[3])
		elif message.is_signal (ibus.IBUS_ENGINE_IFACE, "UpdateAuxString"):
			args = message.get_args_list ()
			self.emit ("update-aux-string", args[0], args[1], args[2])
		elif message.is_signal (ibus.IBUS_ENGINE_IFACE, "UpdateLookupTable"):
			args = message.get_args_list ()
			self.emit ("update-lookup-table", args[0], args[1])
		elif message.is_signal (ibus.IBUS_ENGINE_IFACE, "RegisterProperties"):
			args = message.get_args_list ()
			self.emit ("register-properties", args[0])
		elif message.is_signal (ibus.IBUS_ENGINE_IFACE, "UpdateProperty"):
			args = message.get_args_list ()
			self.emit ("update-property", args[0])
		else:
			return False

		return True

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

	# cursor for lookup table

	def page_up (self):
		self._engine.PageUp ()

	def page_down (self):
		self._engine.PageDown ()
	
	def cursor_up (self):
		self._engine.CursorUp ()
	
	def cursor_down (self):
		self._engine.CursorDown ()

	def property_activate (self, prop_name):
		self._engine.PropertyActivate (prop_name)

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

