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
import gobject
import ibus

class ToolButton (gtk.ToolButton):
	__gsignals__ = {
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_INT)),
		}

	def __init__ (self, label = None, icon = None, prop = None):
		if prop == None:
			prop = ibus.Property (name= "", label = label, icon = icon)
		self._prop = prop

		gtk.ToolButton.__init__ (self, label = prop._label)
		self.set_icon_name (prop._icon)

	def set_icon_name (self, icon_name):
		if icon_name:
			gtk.ToolButton.set_icon_name (self, icon_name)
			self.set_is_important (False)
		else:
			gtk.ToolButton.set_icon_name (self, None)
			self.set_is_important (True)

		self._prop._icon = icon_name

	def do_clicked (self):
		self.emit ("property-activate", self._prop._name, self._prop._state)

gobject.type_register (ToolButton, "ToolButton")

class ToggleToolButton (gtk.ToggleToolButton):
	__gsignals__ = {
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_INT)),
		}

	def __init__ (self, label = None, icon = None, state = ibus.PROP_STATE_UNCHECKED, prop = None):
		if prop == None:
			prop = ibus.Property (name = "", label = label, icon = icon, state = state)
		self._prop = prop

		gtk.ToggleToolButton.__init__ (self)
		self.set_label (prop._label)
		self.set_icon_name (prop._icon)
		self.set_state (prop._state)

	def set_icon_name (self, icon_name):
		if icon_name:
			gtk.ToolButton.set_icon_name (self, icon_name)
			self.set_is_important (False)
		else:
			gtk.ToolButton.set_icon_name (self, None)
			self.set_is_important (True)

		self._prop._icon = icon_name

	def set_state (self, state):
		self.set_active (state == ibus.PROP_STATE_CHECKED)
		self._prop._state = state

	def get_state (self):
		return self._prop._state

	def do_toggled (self):
		if self.get_active ():
			self._prop._state = ibus.PROP_STATE_CHECKED
		else:
			self._prop._state = ibus.PROP_STATE_UNCHECKED
		self.emit ("property-activate", self._prop._name, self._prop._state)

gobject.type_register (ToggleToolButton, "IBusToggleToolButton")

class MenuToolButton (gtk.ToolButton):
	__gsignals__ = {
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_INT)),
		}

	def __init__ (self, label = None, icon = None, prop = None):
		if prop == None:
			prop = ibus.Property (name= "", label = label, icon = icon)
		self._prop = prop

		gtk.ToolButton.__init__ (self, label = prop._label)
		self.set_icon_name (prop._icon)

	def set_icon_name (self, icon_name):
		if icon_name:
			gtk.ToolButton.set_icon_name (self, icon_name)
			self.set_is_important (False)
		else:
			gtk.ToolButton.set_icon_name (self, None)
			self.set_is_important (True)

		self._prop._icon = icon_name

	def do_clicked (self):
		self.emit ("property-activate", self._prop._name, self._prop._state)

gobject.type_register (MenuToolButton, "MenuToolButton")

