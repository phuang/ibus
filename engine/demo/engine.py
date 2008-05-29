#!/usr/bin/env python
import gobject
import gtk
import pango
import dbus
import ibus
from ibus import keysyms
from ibus import interface

class Engine (interface.IEngine):
	def __init__ (self, dbusconn, object_path):
		interface.IEngine.__init__ (self, dbusconn, object_path)
		self._dbusconn = dbusconn
		self._preedit_string = ""

	# methods for dbus rpc
	def ProcessKeyEvent (self, keyval, is_press, state):
		# ignore key release events
		if not is_press:
			return False

		if self._preedit_string:
			if keyval == keysyms.Return:
				self.CommitString (self._preedit_string)
				self._preedit_string = ""
				self._update_preedit ()
				return True
			elif keyval == keysyms.BackSpace:
				self._preedit_string = self._preedit_string[:-1]
				self._update_preedit ()
				return True

		# ignore hotkeys or key > 128
		if keyval >= 128 or (state & (keysyms.CONTROL_MASK | keysyms.MOD1_MASK)) != 0:
			return False

		self._preedit_string += chr (keyval)
		self._update_preedit ()

		return True

	def _update_preedit (self):
		preedit_len = len (self._preedit_string)
		attrs = ibus.AttrList ()
		attrs.append (ibus.AttributeUnderline (pango.UNDERLINE_SINGLE, 0, preedit_len))
		self.PreeditChanged (self._preedit_string, attrs.to_dbus_value (), dbus.Int32 (preedit_len))
		self.AuxStringChanged (self._preedit_string, attrs.to_dbus_value ())

	def FocusIn (self):
		print "FocusIn"

	def FocusOut (self):
		print "FocusOut"

	def Reset (self):
		print "Reset"

	def SetEnable (self, enable):
		self._enable = enable

	def Destroy (self):
		print "Destroy"

	def SetCursorLocation (self, x, y, w, h):
		pass

class DemoEngine (Engine):
	pass

