import gobject
import ibus

class Client (ibus.Object):
	__gsignals__ = {
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

	def __init__ (self, name, ibusconn):
		ibus.Object.__init__ (self)

		self._ibusconn = ibusconn
		self._ibusconn.connect ("destroy", self._ibusconn_destroy_cb)

		# init default values
		self._enable = False
		self._engine = None
		self._engine_handler_ids = []

		# client state
		self._aux_string = None
		self._aux_attrs = None

		self._use_preedit = True
		self._preedit_string = None
		self._preedit_attrs = None
		self._cursor_pos = 0

		self._lookup_table = None
		self._show_lookup_table = False

	def get_preedit_string (self):
		return self._preedit_string, self._preedit_attrs, self._cursor_pos

	def get_use_preedit (self):
		return self._use_preedit

	def get_aux_string (self):
		return self._aux_string, self._aux_attrs

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

	def page_up (self):
		if self._engine:
			self._engine.page_up ()
	
	def page_down (self):
		if self._engine:
			self._engine.page_down ()
	
	def cursor_up (self):
		if self._engine:
			self._engine.cursor_up ()
	
	def cursor_down (self):
		if self._engine:
			self._engine.cursor_down ()

	def property_activate (self, prop_name):
		if self._engine:
			self._engine.property_activate (prop_name)

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

	def update_preedit (self, text, attrs, cursor_pos, visible):
		if self._use_preedit:
			self._ibusconn.emit_dbus_signal ("UpdatePreedit", text, attrs, cursor_pos, visible)
		else:
			# show preedit on panel
			self.emit ("update-preedit", text, attrs, cursor_pos, visible)

	def set_engine (self, engine):
		if self._engine == engine:
			return

		if self._engine != None:
			self._remove_engine_handlers ()
			self._engine.destroy ()
			self._engine = None

		self._engine = engine
		self._install_engine_handlers ()

	def get_engine (self):
		return self._engine

	def _engine_destroy_cb (self, engine):
		if self._engine == engine:
			self._remove_engine_handlers ()
		self._engine = None

	def _ibusconn_destroy_cb (self, ibusconn):
		if self._engine != None:
			self._remove_engine_handlers ()
			self._engine.destroy ()
			self._engine = None

	def _commit_string_cb (self, engine, text):
		self.commit_string (text)

	def _update_preedit_cb (self, engine, text, attrs, cursor_pos, visible):
		self.update_preedit (text, attrs, cursor_pos, visible)

	def _update_aux_string_cb (self, engine, text, attrs, visible):
		self._aux_string = text
		self._aux_attrs = attrs
		self.emit ("update-aux-string", text, attrs, visible)

	def _update_lookup_table_cb (self, engine, lookup_table, visible):
		self._lookup_table = lookup_table
		self.emit ("update-lookup-table", lookup_table, visible)

	def _register_properties_cb (self, engine, props):
		self.emit ("register-properties", props)
	
	def _update_property_cb (self, engine, prop):
		self.emit ("update-property", prop)

	def _remove_engine_handlers (self):
		assert self._engine != None
		for id in self._engine_handler_ids:
			self._engine.disconnect (id)
		self._engine_handler_ids = []

	def _install_engine_handlers (self):
		id = self._engine.connect ("destroy", self._engine_destroy_cb)
		self._engine_handler_ids.append (id)
		id = self._engine.connect ("commit-string", self._commit_string_cb)
		self._engine_handler_ids.append (id)
		id = self._engine.connect ("update-preedit", self._update_preedit_cb)
		self._engine_handler_ids.append (id)
		id = self._engine.connect ("update-aux-string", self._update_aux_string_cb)
		self._engine_handler_ids.append (id)
		id = self._engine.connect ("update-lookup-table", self._update_lookup_table_cb)
		self._engine_handler_ids.append (id)
		id = self._engine.connect ("register-properties", self._register_properties_cb)
		self._engine_handler_ids.append (id)
		id = self._engine.connect ("update-property", self._update_property_cb)
		self._engine_handler_ids.append (id)

gobject.type_register (Client)
