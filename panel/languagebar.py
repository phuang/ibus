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

ICON_SIZE = gtk.ICON_SIZE_MENU

class LanguageBar (gtk.Toolbar):
	__gsignals__ = {
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, )),
		"im-menu-popup" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_PYOBJECT, )),
		}

	def __init__ (self):
		gtk.Toolbar.__init__ (self)
		self.set_property ("icon-size", ICON_SIZE)
		icon_theme = gtk.icon_theme_get_default ()
		icon_theme.prepend_search_path ("/home/phuang/sources/ibus/icons")
		self._create_ui ()

		self._properties = {}
		self._toplevel = gtk.Window (gtk.WINDOW_POPUP)
		self._toplevel.add (self)

		root = gdk.get_default_root_window ()
		workarea = root.property_get ("_NET_WORKAREA")[2]
		self._toplevel.move (workarea[2] - 200, workarea[3] - 40)

	def _add_items (self):
		img = gtk.image_new_from_icon_name ("engine-default", ICON_SIZE)
		btn = gtk.ToolButton (img, "engine")
		btn.connect ("clicked", lambda x: self._add_items ())
		self.insert (btn, -1)

		img = gtk.image_new_from_icon_name ("ibus-keyboard", ICON_SIZE)
		btn = gtk.ToolButton (img, "keyboard")
		self.insert (btn, -1)

		img = gtk.image_new_from_icon_name ("ibus-zh", ICON_SIZE)
		btn = gtk.ToolButton (img, "keyboard")
		self.insert (btn, -1)

		self.insert (gtk.SeparatorToolItem (), -1)
		self.show_all ()
		self.check_resize ()

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
				widget = gtk.ToolButton ()
				widget.set_icon_name (prop._icon)
				widget.set_label (prop._label)
				widget.connect ("clicked",
						lambda widget, prop: self.emit ("property-activate", prop._name),
						prop)
			else:
				widget = gtk.ToolItem ()

			widget.set_sensitive (prop._sensitive)
			if prop._visible:
				widget.set_no_show_all (False)
				widget.show ()
			else:
				widget.set_no_show_all (True)
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

gobject.type_register (LanguageBar, "IBusLanguageBar")

