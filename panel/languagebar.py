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
from image import Image
from handle import Handle
import menu

ICON_SIZE = gtk.ICON_SIZE_MENU

class LanguageBar (gtk.Toolbar):
	__gsignals__ = {
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_INT)),
		"im-menu-popup" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_PYOBJECT, )),
		}

	def __init__ (self):
		gtk.Toolbar.__init__ (self)
		self.set_property ("icon-size", ICON_SIZE)
		self._create_ui ()

		self._properties = {}
		self._toplevel = gtk.Window (gtk.WINDOW_POPUP)
		self._toplevel.add (self)

		root = gdk.get_default_root_window ()
		workarea = root.property_get ("_NET_WORKAREA")[2]
		self._toplevel.move (workarea[2] - 200, workarea[3] - 40)

	def _create_ui (self):
		# create move handle
		self._handle = gtk.ToolItem ()
		self._handle.add (Handle ())
		self.insert (self._handle, -1)

		# create input methods menu
		image = gtk.image_new_from_icon_name ("engine-default", gtk.ICON_SIZE_MENU)
		self._im_menu = gtk.ToolButton (icon_widget = image)
		self._im_menu.connect ("clicked", lambda w: self.emit ("im-menu-popup", self._im_menu))
		self.insert (self._im_menu, -1)

	def _remove_properties (self):
		# reset all properties
		for name, props in self._properties.items ():
			for prop, widget in props:
				widget.hide ()
				widget.destroy ()
		self._properties = {}
		self.check_resize ()

	def do_show (self):
		gtk.Toolbar.do_show (self)
		self.check_resize ()

	def do_check_resize (self):
		width = 0
		for item in self:
			w, h = item.size_request ()
			width += w
		self.set_size_request (width + 4, -1)

	def do_size_request (self, requisition):
		gtk.Toolbar.do_size_request (self, requisition)
		self._toplevel.resize (1, 1)

	def reset (self):
		self._remove_properties ()

	def register_properties (self, props):
		self._remove_properties ()
		# create new properties
		for prop in props:
			if prop._type == ibus.PROP_TYPE_NORMAL:
				widget = gtk.ToolButton (label = prop._label)
				if prop._icon:
					widget.set_icon_name (prop._icon)
				else:
					widget.set_is_important (True)
				widget.connect ("clicked",
						self._property_clicked, prop)
			elif prop._type == ibus.PROP_TYPE_TOGGLE:
				widget = gtk.ToggleToolButton ()
				widget.set_icon_name (prop._icon)
				widget.set_label (prop._label)
				widget.set_active (prop._state == ibus.PROP_STATE_CHECKED)
				widget.connect ("clicked",
						self._property_clicked, prop)
			elif prop._type == ibus.PROP_TYPE_MENU:
				widget = gtk.ToolButton (label = prop._label)
				if prop._icon:
					widget.set_icon_name (prop._icon)
				else:
					widget.set_is_important (True)
				menu = self._create_prop_menu (prop.get_sub_props ())
				widget.connect ("clicked",
						self._property_menu_clicked, prop, menu)
			elif prop._type == PROP_TYPE_SEPARATOR:
				widget = gtk.SeparatorToolItem ()
			else:
				widget = gtk.ToolItem ()

			widget.set_sensitive (prop._sensitive)

			widget.set_no_show_all (True)
			if prop._visible:
				widget.show ()
			else:
				widget.hide ()

			if not self._properties.has_key (prop._name):
				self._properties [prop._name] = []

			self._properties [prop._name].append ((prop, widget))
			self.insert (widget, -1)

		self.check_resize ()

	def update_properties (self, props):
		pass

	def show_all (self):
		self._toplevel.show_all ()
		gtk.Toolbar.show_all (self)

	def hide_all (self):
		self._toplevel.hide_all ()
		gtk.Toolbar.hide_all (self)

	def _create_prop_menu (self, props):
		menu = gtk.Menu ()
		menu.set_take_focus (False)

		radio_group = None

		for prop in props:
			if prop._type == ibus.PROP_TYPE_NORMAL:
				item = gtk.ImageMenuItem (prop._label)
				if prop._icon:
					item.set_image (gtk.image_new_from_icon_name  (prop._icon, gtk.ICON_SIZE_MENU))
				item.connect ("activate", self._property_clicked, prop)
			elif prop._type == ibus.PROP_TYPE_TOGGLE:
				item = gtk.CheckMenuItem (label = prop._label)
				item.set_active (prop._state == ibus.PROP_STATE_CHECKED)
				item.connect ("toggled", self._property_clicked, prop)
			elif prop._type == ibus.PROP_TYPE_RADIO:
				item = gtk.RadioMenuItem (group = radio_group, label = prop._label)
				item.set_active (prop._state == ibus.PROP_STATE_CHECKED)
				if radio_group == None:
					radio_group = item
				item.connect ("toggled", self._property_clicked, prop)
			elif prop._type == ibus.PROP_TYPE_SEPARATOR:
				item = gtk.SeparatorMenuItem ()
				radio_group = None
			elif prop._type == ibus.PROP_TYPE_MENU:
				item = gtk.ImageMenuItem (prop._label)
				if prop._icon:
					item.set_image (gtk.image_new_from_icon_name  (prop._icon, gtk.ICON_SIZE_MENU))
				item.set_submenu (self._create_prop_menu (prop.get_sub_props ()))
			else:
				assert Fasle


			item.set_sensitive (prop._sensitive)
			item.set_no_show_all (True)
			if prop._visible:
				item.show ()
			else:
				item.hide ()

			menu.append (item)

		menu.show_all ()

		return menu

	def _property_clicked (self, widget, prop):
		if prop._type in (ibus.PROP_TYPE_TOGGLE, ibus.PROP_TYPE_RADIO):
			if widget.get_active ():
				prop._state = ibus.PROP_STATE_CHECKED
			else:
				prop._state = ibus.PROP_STATE_UNCHECKED
		self.emit ("property-activate", prop._name, prop._state)

	def _property_menu_clicked (self, widget, prop, menu):
		menu.popup (None, None, menu.menu_position,
					0, gtk.get_current_event_time (), widget)

gobject.type_register (LanguageBar, "IBusLanguageBar")

