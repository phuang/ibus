# -*- coding: utf-8 -*-
import gtk
import gtk.gdk as gdk
import gobject
import ibus
from ibus import interface
from languagebar import LanguageBarWindow
from candidatewindow import CandidateWindow

class Panel (gobject.GObject):
	def __init__ (self, proxy):
		gobject.GObject.__init__ (self)
		self._proxy = proxy
		self._language_bar = LanguageBarWindow ()
		self._candidate_panel = CandidateWindow ()

	def set_cursor_location (self, x, y, w, h):
		self._candidate_panel.move (x + w, y + h)
	
	def set_preedit_string (self, text, attrs, cursor_pos):
		self._candidate_panel.set_preedit_string (text, attrs, cursor_pos)

	def show_preedit_string (self):
		self._candidate_panel.show_preedit_string ()

	def hide_preedit_string (self):
		self._candidate_panel.hide_preedit_string ()

	def set_aux_string (self, text, attrs):
		self._candidate_panel.set_aux_string (text, attrs)

	def show_aux_string (self):
		self._candidate_panel.show_aux_string ()

	def hide_aux_string (self):
		self._candidate_panel.hide_aux_string ()
	
	def update_lookup_table (self, lookup_table):
		self._candidate_panel.set_lookup_table (lookup_table)

	def show_candidate_window (self):
		self._candidate_panel.show ()

	def hide_candidate_window (self):
		self._candidate_panel.hide ()

	def show_language_bar (self):
		selk._language_bar.show ()

	def hide_language_bar (self):
		selk._language_bar.hide ()

class PanelProxy (interface.IPanel):
	def __init__ (self, dbusconn, object_path):
		interface.IPanel.__init__ (self, dbusconn, object_path)
		self._dbusconn = dbusconn
		self._panel = Panel (self)

	def SetCursorLocation (self, x, y, w, h):
		self._panel.set_cursor_location (x, y, w, h)
	
	def SetPreeditString (self, text, attrs, cursor_pos):
		attrs = ibus.attr_list_from_dbus_value (attrs)
		self._panel.set_preedit_string (text, atrrs, cursor_pos)

	def ShowPreeditString (self):
		self._panel.show_preedit_string ()

	def HidePreeditString (self):
		self._panel.hide_preedit_string ()

	def SetAuxString (self, text, attrs):
		attrs = ibus.attr_list_from_dbus_value (attrs)
		self._panel.set_aux_string (text, attrs)

	def ShowAuxString (self):
		self._panel.show_aux_string ()

	def HideAuxString (self):
		self._panel.hide_aux_string ()

	def UpdateLookupTable (self, lookup_table):
		self._panel.update_lookup_table ()

	def ShowCandidateWindow (self):
		self._panel.show_candidate_window ()

	def HideCandidateWindow (self):
		self._panel.hide_candidate_window ()

	def ShowLanguageBar (self):
		self._panel.show_language_bar ()

	def HideLanguageBar (self):
		self._panel.hide_language_bar ()

	def Destroy (self):
		self._pabel.destroy ()

