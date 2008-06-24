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

class Menu (gtk.Menu):
	__gsignals__ = {
	"property-activate" : (
		gobject.SIGNAL_RUN_FIRST,
		gobject.TYPE_NONE,
		(gobject.TYPE_STRING, gobject.TYPE_INT)),
	}

	def __init__ (self, prop):
		self._prop = prop
		self._items = []
		gtk.Menu.__init__ (self)
		self.set_take_focus (False)
		self._create_items (self._prop.get_sub_props ())
		self.show_all ()

	def _create_items (self, props):
		radio_group = None

		for prop in props:
			if prop._type == ibus.PROP_TYPE_NORMAL:
				item = gtk.ImageMenuItem (prop)
			elif prop._type == ibus.PROP_TYPE_TOGGLE:
				item = CheckMenuItem (prop)
			elif prop._type == ibus.PROP_TYPE_RADIO:
				item = RadioMenuItem (radio_group, prop)
				radio_group = item
			elif prop._type == ibus.PROP_TYPE_SEPARATOR:
				item = SeparatorMenuItem ()
				radio_group = None
			elif prop._type == ibus.PROP_TYPE_MENU:
				item = gtk.ImageMenuItem (prop)
				item.set_submenu (Menu (prop))
			else:
				assert Fasle

			item.set_sensitive (prop._sensitive)
			item.set_no_show_all (True)
			if prop._visible:
				item.show ()
			else:
				item.hide ()

			item.connect ("property-activate", lambda w,n,s: self.emit ("property-activate", n, s))

			self.append (item)
			self._items.append (item)

	def popup (self, button, active_time, widget):
		gtk.Menu.popup (self, None, None, menu_position,
							button, active_time, widget)

	def _property_clicked (self, item, prop):
		pass


class ImageMenuItem (gtk.ImageMenuItem):
	__gsignals__ = {
	"property-activate" : (
		gobject.SIGNAL_RUN_FIRST,
		gobject.TYPE_NONE,
		(gobject.TYPE_STRING, gobject.TYPE_INT)),
	}

	def __init__ (self, prop):
		self._prop = prop
		gtk.ImageMenuItem.__init__ (self, label = prop._label)
		if prop._icon:
			self.set_image (gtk.image_new_from_icon_name  (prop._icon, gtk.ICON_SIZE_MENU))

	def do_activate (self):
		self.emit ("property-activate", self._prop._name, self._prop._state)


class CheckMenuItem (gtk.CheckMenuItem):
	__gsignals__ = {
	"property-activate" : (
		gobject.SIGNAL_RUN_FIRST,
		gobject.TYPE_NONE,
		(gobject.TYPE_STRING, gobject.TYPE_INT)),
	}

	def __init__ (self, prop):
		self._prop = prop
		gtk.CheckMenuItem.__init__ (self, label = prop._label)
		self.set_active (prop._state == ibus.PROP_STATE_CHECKED)

	def do_toggled (self):
		if self.get_active ():
			self._prop._state = ibus.PROP_STATE_CHECKED
		else:
			self._prop._state = ibus.PROP_STATE_UNCHECKED
		self.emit ("property-activate", self._prop._name, self._prop._state)


class RadioMenuItem (gtk.RadioMenuItem):
	__gsignals__ = {
	"property-activate" : (
		gobject.SIGNAL_RUN_FIRST,
		gobject.TYPE_NONE,
		(gobject.TYPE_STRING, gobject.TYPE_INT)),
	}

	def __init__ (self, group, prop):
		self._prop = prop
		gtk.RadioMenuItem.__init__ (self, group, label = prop._label)
		self.set_active (prop._state == ibus.PROP_STATE_CHECKED)

	def do_toggled (self):
		if self.get_active ():
			self._prop._state = ibus.PROP_STATE_CHECKED
		else:
			self._prop._state = ibus.PROP_STATE_UNCHECKED
		self.emit ("property-activate", self._prop._name, self._prop._state)

class SeparatorMenuItem (gtk.SeparatorMenuItem):
	__gsignals__ = {
	"property-activate" : (
		gobject.SIGNAL_RUN_FIRST,
		gobject.TYPE_NONE,
		(gobject.TYPE_STRING, gobject.TYPE_INT)),
	}


gobject.type_register (Menu, "IBusMenu")
gobject.type_register (ImageMenuItem, "IBusImageMenuItem")
gobject.type_register (CheckMenuItem, "IBusCheckMenuItem")
gobject.type_register (RadioMenuItem, "IBusRadioMenuItem")
gobject.type_register (SeparatorMenuItem, "IBusSeparatorMenuItem")

def menu_position (menu, button):
	screen = button.get_screen ()
	monitor = screen.get_monitor_at_window (button.window)
	monitor_allocation = screen.get_monitor_geometry (monitor)

	x, y = button.window.get_origin ()
	x += button.allocation.x
	y += button.allocation.y

	menu_width, menu_height = menu.size_request ()

	if x + menu_width >= monitor_allocation.width:
		x -= menu_width - button.allocation.width
	elif x - menu_width <= 0:
		pass
	else:
		if x <= monitor_allocation.width * 3 / 4:
			pass
		else:
			x -= menu_width - button.allocation.width

	if y + button.allocation.height + menu_height >= monitor_allocation.height:
		y -= menu_height
	elif y - menu_height <= 0:
		y += button.allocation.height
	else:
		if y <= monitor_allocation.height * 3 / 4:
			y += button.allocation.height
		else:
			y -= menu_height

	return (x, y, False)

