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
from menu import menu_position
from toolitem import ToolButton,\
	ToggleToolButton, \
	SeparatorToolItem, \
	MenuToolButton

ICON_SIZE = gtk.ICON_SIZE_MENU

class LanguageBar (gtk.Toolbar):
	__gsignals__ = {
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_INT)),
		"get-im-menu" : (
			gobject.SIGNAL_RUN_LAST,
			gobject.TYPE_PYOBJECT,
			()),
		}

	def __init__ (self):
		gtk.Toolbar.__init__ (self)
		self.set_style (gtk.TOOLBAR_ICONS)
		self.set_property ("icon-size", ICON_SIZE)
		self._create_ui ()

		self._properties = []
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
		self._im_menu = ToggleToolButton (ibus.Property (name = "", type = ibus.PROP_TYPE_TOGGLE, icon = "engine-default", tooltip = "Swicth engine"))
		self._im_menu.connect ("toggled", self._im_menu_toggled_cb)
		self.insert (self._im_menu, -1)

	def _im_menu_toggled_cb (self, widget):
		if self._im_menu.get_active ():
			menu = self.emit ("get-im-menu")
			menu.connect ("deactivate", self._im_menu_deactivate_cb)
			menu.popup (None, None,
				menu_position,
				0,
				gtk.get_current_event_time (),
				widget)
	def _im_menu_deactivate_cb (self, menu):
		self._im_menu.set_active (False)

	def _remove_properties (self):
		# reset all properties

		map (lambda i: i.destroy (), self._properties)
		self._properties = []
		self.check_resize ()

	def do_show (self):
		gtk.Toolbar.do_show (self)
		self.check_resize ()

	def do_check_resize (self):
		width = 0
		for item in self:
			w, h = item.size_request ()
			width += w
		self.set_size_request (width + 32, -1)

	def do_size_request (self, requisition):
		gtk.Toolbar.do_size_request (self, requisition)
		self._toplevel.resize (1, 1)

	def set_im_icon (self, icon_name):
		self._im_menu.set_icon_name (icon_name)

	def reset (self):
		self._remove_properties ()

	def register_properties (self, props):
		self._remove_properties ()
		# create new properties
		for prop in props:
			if prop._type == ibus.PROP_TYPE_NORMAL:
				item = ToolButton (prop = prop)
			elif prop._type == ibus.PROP_TYPE_TOGGLE:
				item = ToggleToolButton (prop = prop)
			elif prop._type == ibus.PROP_TYPE_MENU:
				item = MenuToolButton (prop = prop)
			elif prop._type == PROP_TYPE_SEPARATOR:
				item = SeparatorToolItem ()
			else:
				raise IBusException ("Unknown property type = %d" % prop._type)

			item.connect ("property-activate",
						lambda w, n, s: self.emit ("property-activate", n, s))
			item.connect ("size-request", lambda w, s: self.check_resize ())

			item.set_sensitive (prop._sensitive)

			item.set_no_show_all (True)

			if prop._visible:
				item.show ()
			else:
				item.hide ()

			self._properties.append (item)
			self.insert (item, -1)

		self.check_resize ()

	def update_property (self, prop):
		map (lambda x: x.update_property (prop), self._properties)

	def show_all (self):
		self._toplevel.show_all ()
		gtk.Toolbar.show_all (self)

	def hide_all (self):
		self._toplevel.hide_all ()
		gtk.Toolbar.hide_all (self)

	def focus_in (self):
		self._im_menu.set_sensitive (True)

	def focus_out (self):
		self._im_menu.set_sensitive (False)

gobject.type_register (LanguageBar, "IBusLanguageBar")

