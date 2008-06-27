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

import gtk
import gtk.gdk as gdk
import gobject
import ibus
from menu import *

class ToolItem:
	def __init__ (self, prop):
		self._prop = prop
		self._sub_itmes = []

	def update_property (self, prop):
		if self._prop == None:
			return False

		retval = False

		if self._prop._name == prop._name and self._prop._type == prop._type:
			self._prop = prop
			self.property_changed ()
			retval =  True

		if any (map (lambda i: i.update_property (prop), self._sub_itmes)):
			retval = True

		return retval

	def set_prop_label (self, label):
		self._prop._label = label
		self.property_changed ()

	def set_icon (self, icon):
		self._prop._icon = icon
		self.property_changed ()

	def set_tooltip (self, tooltip):
		self._prop._tooltip = tooltip
		self.property_changed ()

	def set_state (self, state):
		self._prop._state = state
		self.property_changed ()

	def property_changed (self):
		pass


class ToolButton (gtk.ToolButton, ToolItem):
	__gsignals__ = {
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_INT)),
		}

	def __init__ (self, prop):
		gtk.ToolButton.__init__ (self, label = prop._label)
		ToolItem.__init__ (self, prop)

		self.set_icon_name (prop._icon)
		self.set_tooltip_text (prop._tooltip)
		self.set_sensitive (prop._sensitive)

		if prop._visible:
			self.set_no_show_all (False)
			self.show_all ()
		else:
			self.set_no_show_all (True)
			self.hide_all ()


	def set_icon_name (self, icon_name):
		if icon_name:
			gtk.ToolButton.set_icon_name (self, icon_name)
			self.set_is_important (False)
		else:
			gtk.ToolButton.set_icon_name (self, None)
			self.set_is_important (True)

		self._prop._icon = icon_name

	def set_tooltip_text (self, text):
		if text:
			gtk.ToolButton.set_tooltip_text (self, text)
		else:
			gtk.ToolButton.set_tooltip_text (self, None)

		self._prop._tooltip = text

	def property_changed (self):
		self.set_icon_name (self._prop._icon)
		self.set_tooltip_text (self._prop._tooltip)
		self.set_label (self._prop._label)

	def do_clicked (self):
		self.emit ("property-activate", self._prop._name, self._prop._state)


class ToggleToolButton (gtk.ToggleToolButton, ToolItem):
	__gsignals__ = {
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_INT)),
		}

	def __init__ (self, prop):
		gtk.ToggleToolButton.__init__ (self)
		ToolItem.__init__ (self, prop)

		self.set_label (prop._label)
		self.set_icon_name (prop._icon)
		self.set_tooltip_text (prop._tooltip)
		self.set_state (prop._state)
		self.set_sensitive (prop._sensitive)
		if prop._visible:
			self.set_no_show_all (False)
			self.show_all ()
		else:
			self.set_no_show_all (True)
			self.hide_all ()

	def set_icon_name (self, icon_name):
		if icon_name:
			gtk.ToggleToolButton.set_icon_name (self, icon_name)
			self.set_is_important (False)
		else:
			gtk.ToggleToolButton.set_icon_name (self, None)
			self.set_is_important (True)

		self._prop._icon = icon_name

	def set_tooltip_text (self, text):
		if text:
			gtk.ToggleToolButton.set_tooltip_text (self, text)
		else:
			gtk.ToggleToolButton.set_tooltip_text (self, None)

		self._prop._tooltip = text

	def property_changed (self):
		self.set_icon_name (self._prop._icon)
		self.set_tooltip_text (self._prop._tooltip)
		self.set_label (self._prop._label)
		self.set_active (self._prop._state == ibus.PROP_STATE_CHECKED)

	def do_toggled (self):
		if self.get_active ():
			self._prop._state = ibus.PROP_STATE_CHECKED
		else:
			self._prop._state = ibus.PROP_STATE_UNCHECKED
		self.emit ("property-activate", self._prop._name, self._prop._state)

class SeparatorToolItem (gtk.SeparatorToolItem, ToolItem):
	def __init__ (self, prop):
		gtk.SeparatorToolItem.__init__ (self)
		ToolItem.__init__ (self, prop)

class MenuToolButton (ToggleToolButton):
	# __gsignals__ = {
	#		"property-activate" : (
	#			gobject.SIGNAL_RUN_FIRST,
	#			gobject.TYPE_NONE,
	#			(gobject.TYPE_STRING, gobject.TYPE_INT)),
	#		}

	def __init__ (self, prop):
		ToggleToolButton.__init__ (self, prop)
		self._menu = Menu (prop)
		self._menu.connect ("deactivate", lambda m: self.set_active (False))
		self._menu.connect ("property-activate", lambda w,n,s: self.emit ("property-activate", n, s))

	def do_toggled (self):
		if self.get_active ():
			self._menu.popup (0, gtk.get_current_event_time (), self)

gobject.type_register (ToolButton, "ToolButton")
gobject.type_register (ToggleToolButton, "IBusToggleToolButton")
gobject.type_register (MenuToolButton, "MenuToolButton")

