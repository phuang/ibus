import weakref
import gobject
import ibus

class Panel (ibus.Object):
	__gsignals__ = {
		"page-up" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"page-down" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"cursor-up" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"cursor-down" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, )),
	}

	def __init__ (self, ibusconn, object_path):
		ibus.Object.__init__ (self)
		self._ibusconn = ibusconn
		self._object_path = object_path
		self._panel = self._ibusconn.get_object (self._object_path)

		self._ibusconn.connect ("destroy", self._ibusconn_destroy_cb)
		self._ibusconn.connect ("dbus-signal", self._dbus_signal_cb)

	def set_cursor_location (self, x, y, w, h):
		self._panel.SetCursorLocation (x, y, w, h,
				reply_handler = ibus.default_reply_handler,
				error_handler = ibus.default_error_handler)

	def update_preedit (self, text, attrs, cursor_pos, visible):
		self._panel.UpdatePreedit (text, attrs, cursor_pos, visible,
				reply_handler = ibus.default_reply_handler,
				error_handler = ibus.default_error_handler)

	def update_aux_string (self, text, attrs, visible):
		self._panel.UpdateAuxString (text, attrs, visible,
				reply_handler = ibus.default_reply_handler,
				error_handler = ibus.default_error_handler)

	def update_lookup_table (self, lookup_table, visible):
		self._panel.UpdateLookupTable (lookup_table, visible,
				reply_handler = ibus.default_reply_handler,
				error_handler = ibus.default_error_handler)

	def register_properties (self, props):
		self._panel.RegisterProperties (props,
				reply_handler = ibus.default_reply_handler,
				error_handler = ibus.default_error_handler)

	def update_property (self, prop):
		self._panel.UpdateProperties (prop,
				reply_handler = ibus.default_reply_handler,
				error_handler = ibus.default_error_handler)

	def show_language_bar (self):
		self._panel.ShowLanguageBar (
				reply_handler = ibus.default_reply_handler,
				error_handler = ibus.default_error_handler)

	def hide_language_bar (self):
		self._panel.HideLanguageBar (
				reply_handler = ibus.default_reply_handler,
				error_handler = ibus.default_error_handler)

	def reset (self):
		self._panel.Reset (
				reply_handler = ibus.default_reply_handler,
				error_handler = ibus.default_error_handler)

	def destroy (self):
		if self._ibusconn != None:
			self._panel.Destroy (
					reply_handler = ibus.default_reply_handler,
					error_handler = ibus.default_error_handler)

		self._ibusconn = None
		self._panel = None
		ibus.Object.destroy (self)

	# signal callbacks
	def _ibusconn_destroy_cb (self, ibusconn):
		self._ibusconn = None
		self.destroy ()

	def _dbus_signal_cb (self, ibusconn, message):
		if message.is_signal (ibus.IBUS_PANEL_IFACE, "PageUp"):
			self.emit ("page-up")
		elif message.is_signal (ibus.IBUS_PANEL_IFACE, "PageDown"):
			self.emit ("page-down")
		elif message.is_signal (ibus.IBUS_PANEL_IFACE, "CursorUp"):
			self.emit ("cursor-up")
		elif message.is_signal (ibus.IBUS_PANEL_IFACE, "CursorDown"):
			self.emit ("cursor-down")
		elif message.is_signal (ibus.IBUS_PANEL_IFACE, "PropertyActivate"):
			args = message.get_args_list ()
			self.emit ("property-activate", args[0])
		else:
			return False
		return True

	# methods for cmp
	# def __lt__ (self, other):
	#		x = self.get_info ()
	#		y = other.get_info ()
	#		if x[1] < y[1]: return True
	#		if x[1] == y[1]: return x[0] < y[0]
	#
	#	def __gt__ (self, other):
	#		x = self.get_info ()
	#		y = other.get_info ()
	#		if x[1] > y[1]: return True
	#		if x[1] == y[1]: return x[0] > y[0]

gobject.type_register (Panel)

class DummyPanel (ibus.Object):
	__gsignals__ = {
		"page-up" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"page-down" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"cursor-up" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"cursor-down" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, )),
	}

	def set_cursor_location (self, x, y, w, h):
		pass

	def update_preedit (self, text, attrs, cursor_pos, visible):
		pass

	def update_aux_string (self, text, attrs, visible):
		pass

	def update_lookup_table (self, lookup_table, visible):
		pass

	def register_properties (self, props):
		pass

	def update_property (self, prop):
		pass

	def show_language_bar (self):
		pass

	def hide_language_bar (self):
		pass

	def reset (self):
		pass

gobject.type_register (DummyPanel)
