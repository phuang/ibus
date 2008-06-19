# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA

import gobject
import gtk
import pango
import dbus
import ibus
import enchant
from ibus import keysyms
from ibus import modifier
from ibus import interface

class Engine (interface.IEngine):
	_dict = enchant.Dict ()
	def __init__ (self, dbusconn, object_path):
		interface.IEngine.__init__ (self, dbusconn, object_path)
		self._dbusconn = dbusconn
		self._is_invalidate = False
		self._preedit_string = u""
		self._lookup_table = ibus.LookupTable ()
		self._prop_list = ibus.PropList ()
		self._prop_list.append (ibus.Property ("test", icon = "ibus-locale"))

	def _process_key_event (self, keyval, is_press, state):
		# ignore key release events
		if not is_press:
			return False

		if self._preedit_string:
			if keyval == keysyms.Return:
				self._commit_string (self._preedit_string)
				return True
			elif keyval == keysyms.Escape:
				self._preedit_string = u""
				self._update ()
				return True
			elif keyval == keysyms.BackSpace:
				self._preedit_string = self._preedit_string[:-1]
				self._invalidate ()
				return True
			elif keyval == keysyms.space:
				if self._lookup_table.get_number_of_candidates () > 0:
					self._commit_string (self._lookup_table.get_current_candidate ()[0])
				else:
					self._commit_string (self._preedit_string)
				return False
			elif keyval >= keysyms._1 and keyval <= keysyms._9:
				index = keyval - keysyms._1
				candidates = self._lookup_table.get_canidates_in_current_page ()
				if index >= len (candidates):
					return False
				candidate = candidates[index][0]
				self._commit_string (candidate)
				return True
			elif keyval == keysyms.Page_Up or keyval == keysyms.KP_Page_Up:
				if self._lookup_table.page_up ():
					self._update_lookup_table ()
				return True
			elif keyval == keysyms.Up:
				self._cursor_up ()
				return True
			elif keyval == keysyms.Down:
				self._cursor_down ()
				return True
			elif keyval == keysyms.Left or keyval == keysyms.Right:
				return True
			elif keyval == keysyms.Page_Down or keyval == keysyms.KP_Page_Down:
				if self._lookup_table.page_down ():
					self._update_lookup_table ()
				return True
		if keyval in xrange (keysyms.a, keysyms.z + 1) or \
			keyval in xrange (keysyms.A, keysyms.Z + 1):
			if state & (modifier.CONTROL_MASK | modifier.ALT_MASK) == 0:
				self._preedit_string += unichr (keyval)
				self._invalidate ()
				return True
		else:
			if keyval < 128 and self._preedit_string:
				self._commit_string (self._preedit_string)

		return False

	def _invalidate (self):
		if self._is_invalidate:
			return
		self._is_invalidate = True
		gobject.idle_add (self._update, priority = gobject.PRIORITY_LOW)

	def _cursor_up (self):
		if self._lookup_table.cursor_up ():
			self._update_lookup_table ()
			return True
		return False

	def _cursor_down (self):
		if self._lookup_table.cursor_down ():
			self._update_lookup_table ()
			return True
		return False

	def _commit_string (self, text):
		self.CommitString (text)
		self._preedit_string = u""
		self._update ()

	def _update (self):
		preedit_len = len (self._preedit_string)
		attrs = ibus.AttrList ()
		self._lookup_table.clean ()
		if preedit_len > 0:
			if not self._dict.check (self._preedit_string):
				attrs.append (ibus.AttributeForeground (0xff0000, 0, preedit_len))
				for text in self._dict.suggest (self._preedit_string):
					self._lookup_table.append_candidate (text)
		self.UpdateAuxString (self._preedit_string, attrs.to_dbus_value (), preedit_len > 0)
		attrs.append (ibus.AttributeUnderline (pango.UNDERLINE_SINGLE, 0, preedit_len))
		self.UpdatePreedit (self._preedit_string, attrs.to_dbus_value (), dbus.Int32 (preedit_len), preedit_len > 0)
		self._update_lookup_table ()
		self._is_invalidate = False

	def _update_lookup_table (self):
		show = self._lookup_table.get_number_of_candidates () > 0
		self.UpdateLookupTable (self._lookup_table.to_dbus_value (), show)


	# methods for dbus rpc
	def ProcessKeyEvent (self, keyval, is_press, state):
		try:
			return self._process_key_event (keyval, is_press, state)
		except Exception, e:
			print e
		return False

	def FocusIn (self):
		self.RegisterProperties (self._prop_list.to_dbus_value ())
		print "FocusIn"

	def FocusOut (self):
		print "FocusOut"

	def SetCursorLocation (self, x, y, w, h):
		pass

	def Reset (self):
		print "Reset"

	def PageUp (self):
		print "PageUp"

	def PageDown (self):
		print "PageDown"

	def CursorUp (self):
		self._cursor_up ()

	def CursorDown (self):
		self._cursor_down ()

	def SetEnable (self, enable):
		self._enable = enable
		if self._enable:
			self.RegisterProperties (self._prop_list.to_dbus_value ())

	def PropertyActivate (self, prop_name):
		print "PropertyActivate (%s)" % prop_name

	def Destroy (self):
		print "Destroy"

class DemoEngine (Engine):
	pass

