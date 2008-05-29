import gobject
import ibus

class Client (ibus.Object):
	__gsignals__ = {
		"preedit-changed" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_PYOBJECT, gobject.TYPE_INT)),
		"aux-string-changed" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_PYOBJECT)),
		"update-lookup-table" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_PYOBJECT, )),
		"show-lookup-table" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"hide-lookup-table" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			())
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

	def preedit_changed (self, text, attrs, cursor_pos):
		if self._use_preedit:
			self._ibusconn.emit_dbus_signal ("PreeditChanged", text, attrs, cursor_pos)
		else:
			# show preedit on panel
			self.emit ("preedit-changed", text, attrs, cursor_pos)

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

	def _preedit_changed_cb (self, engine, text, attrs, cursor_pos):
		self.preedit_changed (text, attrs, cursor_pos)

	def _aux_string_changed_cb (self, engine, text, attrs):
		self._aux_string = text
		self._aux_attrs = attrs
		self.emit ("aux-string-changed", text, attrs)

	def _update_lookup_table_cb (self, engine, lookup_table):
		self._lookup_table = lookup_table
		self.emit ("update-lookup-table", lookup_table)

	def _show_lookup_table_cb (self, engine):
		self._show_lookup_table = True
		self.emit ("show-lookup-table")

	def _hide_lookup_table_cb (self, engine):
		self._show_lookup_table = False
		self.emit ("hide-lookup-table")

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
		id = self._engine.connect ("preedit-changed", self._preedit_changed_cb)
		self._engine_handler_ids.append (id)
		id = self._engine.connect ("aux-string-changed", self._aux_string_changed_cb)
		self._engine_handler_ids.append (id)
		id = self._engine.connect ("update-lookup-table", self._update_lookup_table_cb)
		self._engine_handler_ids.append (id)
		id = self._engine.connect ("show-lookup-table", self._show_lookup_table_cb)
		self._engine_handler_ids.append (id)
		id = self._engine.connect ("hide-lookup-table", self._hide_lookup_table_cb)
		self._engine_handler_ids.append (id)
gobject.type_register (Client)
