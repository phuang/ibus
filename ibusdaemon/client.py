import ibus

class Client (ibus.Object):
	def __init__ (self, name, ibusconn):
		ibus.Object.__init__ (self)
		
		self._ibusconn = ibusconn
		self._ibusconn.connect ("destroy", self._ibusconn_destroy_cb)
		
		# init default values
		self._enable = False
		self._factory = None
		self._engine = None
		self._engine_handler_ids = []

	def process_key_event (self, keyval, is_press, state,
								reply_cb, error_cb):
		if self._engine != None and self._enable:
			self._engine.process_key_event (keyval, is_press, state, 
								reply_cb, error_cb)
		else:
			reply_cb (False)

	def set_cursor_location (self, x, y, w, h):
		if self._engine:
			self._engine.set_cursor_location (x, y, w, h)

	def focus_in (self):
		if self._engine:
			self._engine.focus_in ()

	def focus_out (self):
		if self._engine:
			self._engine.focus_out ()

	def reset (self):
		if self._engine:
			self._engine.reset ()

	def is_enabled (self):
		return self._enable

	def set_enable (self, enable):
		if self._enable != enable:
			self._enable = enable
			if self._enable:
				self._ibusconn.emit_dbus_signal ("Enabled")
			else:
				self._ibusconn.emit_dbus_signal ("Disabled")
			if self._engine:
				self._engine.set_enable (self._enable)

	def commit_string (self, text):
		self._ibusconn.emit_dbus_signal ("CommitString", text)

	def preedit_changed (self, text, attrs, cursor):
		self._ibusconn.emit_dbus_signal ("PreeditChanged", text, attrs.get_array (), cursor)

	def set_engine_factory (self, factory):
		if self._factory == factory:
			return
		
		if self._engine != None:
			self._remove_engine_handlers ()
			self._engine.destroy ()
			self._engine = None

		self._factory = factory

		if self._factory:
			self._engine = self._factory.create_engine ()
			self._install_engine_handlers ()

	def get_engine_factory (self):
		return self._factory

	def _engine_destroy_cb (self, engine):
		if self._engine == engine:
			self._remove_engine_handlers ()
		self._engine = None
		self._factory = None

	def _ibusconn_destroy_cb (self, ibusconn):
		self._factory = None
		if self._engine != None:
			self._remove_engine_handlers ()
			self._engine.destroy ()
			self._engine = None

	def _commit_string_cb (self, engine, text):
		self.commit_string (text)

	def _preedit_changed_cb (self, engine, text, attrs, cursor):
		self.preedit_changed (self, text, attrs, cursor)

	def _remove_engine_handlers (self):
		assert self._engine != None
		for id in self._engine_handler_ids:
			self._engine.disconnect (id)
		self._engine_handler_ids = []

	def _install_engine_handlers (self):
		id = self._engine.connect ("destroy", self._engine_destroy_cb)
		id = self._engine.connect ("commit-string", self._commit_string_cb)
		self._engine_handler_ids.append (id)
		id = self._engine.connect ("preedit-changed", self._preedit_changed_cb)
		self._engine_handler_ids.append (id)
