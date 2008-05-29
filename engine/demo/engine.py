#!/usr/bin/env python
import gobject
import gtk
import pango
import dbus
import ibus
import enchant
from ibus import keysyms
from ibus import interface

class Engine (interface.IEngine):
	_dict = enchant.Dict ()
	def __init__ (self, dbusconn, object_path):
		interface.IEngine.__init__ (self, dbusconn, object_path)
		self._dbusconn = dbusconn
		self._preedit_string = ""
		self._lookup_table = ibus.LookupTable ()

	# methods for dbus rpc
	def ProcessKeyEvent (self, keyval, is_press, state):
		# ignore key release events
		if not is_press:
			return False

		if self._preedit_string:
			if keyval == keysyms.Return:
				self.CommitString (self._preedit_string)
				self._preedit_string = ""
				self._update ()
				return True
			elif keyval == keysyms.BackSpace:
				self._preedit_string = self._preedit_string[:-1]
				self._update ()
				return True
			elif keyval == keysyms.space:
				self.CommitString (self._preedit_string)
				self._preedit_string = ""
				self._update ()
				return False
			elif keyval >= keysyms._1 and keyval <= keysyms._9:
				index = keyval - keysyms._1
				candidates = self._lookup_table.get_canidates_in_current_page ()
				if index >= len (candidates):
					return False
				candidate = candidates[index][0]
				self.CommitString (candidate + " ")
				self._preedit_string = ""
				self._update ()
				return True
			elif keyval == keysyms.Page_Up or keyval == keysyms.KP_Page_Up:
				if self._lookup_table.page_up ():
					self._update_lookup_table ()
				return True
			elif keyval == keysyms.Page_Down or keyval == keysyms.KP_Page_Down:
				if self._lookup_table.page_down ():
					self._update_lookup_table ()
				return True
		if (keyval >= keysyms.a and keyval <= keysyms.z) or \
			(keyval >= keysyms.A and keyval <= keysyms.Z):
			if state & (keysyms.CONTROL_MASK | keysyms.ALT_MASK) == 0:
				self._preedit_string += chr (keyval)
				self._update ()
				return True

		return False

	def _update (self):
		preedit_len = len (self._preedit_string)
		attrs = ibus.AttrList ()
		self._lookup_table.clean ()
		if preedit_len > 0:
			if not self._dict.check (self._preedit_string):
				attrs.append (ibus.AttributeForeground (0xff0000, 0, preedit_len))
				for text in self._dict.suggest (self._preedit_string):
					self._lookup_table.append_candidate (text)

		attrs.append (ibus.AttributeUnderline (pango.UNDERLINE_SINGLE, 0, preedit_len))
		self.PreeditChanged (self._preedit_string, attrs.to_dbus_value (), dbus.Int32 (preedit_len))
		self.AuxStringChanged (self._preedit_string, attrs.to_dbus_value ())
		self._update_lookup_table ()

	def _update_lookup_table (self):
		self.UpdateLookupTable (self._lookup_table.to_dbus_value ())


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

