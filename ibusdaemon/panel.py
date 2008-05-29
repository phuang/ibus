import weakref
import gobject
import ibus
from ibus import interface

class Panel (ibus.Object):
	def __init__ (self, ibusconn, object_path):
		ibus.Object.__init__ (self)
		self._ibusconn = ibusconn
		self._object_path = object_path
		self._panel = self._ibusconn.get_object (self._object_path)

		self._ibusconn.connect ("destroy", self._ibusconn_destroy_cb)
		self._ibusconn.connect ("dbus-signal", self._dbus_signal_cb)

	def set_preedit_string (self, text, attrs, cursor_pos):
		self._panel.SetPreeditString (text, attrs, cursor_pos)

	def show_preedit_string (self):
		self._panel.ShowPreeditString ()

	def hide_preedit_string (self):
		slef._panel.HidePreeditString ()

	def set_aux_string (self, text, attrs):
		self._panel.SetAuxString (text, attrs)

	def show_aux_string (self):
		self._panel.ShowAuxString ()

	def hide_aux_string (self):
		slef._panel.HideAuxString ()

	def update_lookup_table (self, lookup_table):
		self._panel.UpdateLookupTable (lookup_table)

	def show_candidate_window (self):
		self._panel.ShowCandidateWindow ()

	def hide_candidate_window (self):
		self._panel.HideCandidateWindow ()

	def show_language_bar (self):
		self._panel.ShowLanguageBar ()

	def hide_language_bar (self):
		self._panel.HideLanguageBar ()

	def destroy (self):
		if self._ibusconn != None:
			self._panel.Destroy ()

		self._ibusconn = None
		self._panel = None
		ibus.Object.destroy (self)

	# signal callbacks
	def _ibusconn_destroy_cb (self, ibusconn):
		self._ibusconn = None
		self.destroy ()

	def _dbus_signal_cb (self, ibusconn, message):
		if message.is_signal (interface.IBUS_PANEL_IFACE, "PageUp"):
			pass
		elif message.is_signal (interface.IBUS_PANEL_IFACE, "PageDown"):
			pass
		elif message.is_signal (interface.IBUS_PANEL_IFACE, "CursorBack"):
			pass
		elif message.is_signal (interface.IBUS_PANEL_IFACE, "CursorForward"):
			pass

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

class DummyPanel:
	pass
