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
from os import path
from lang import LANGUAGES
from ibus import interface
from languagebar import LanguageBar
from candidatepanel import CandidatePanel

class Panel (ibus.Object):
	def __init__ (self, proxy, _ibus):
		gobject.GObject.__init__ (self)
		self._proxy = proxy
		self._ibus = _ibus
		self._focus_ic = None

		# add icon search path
		icon_theme = gtk.icon_theme_get_default ()
		dir = path.dirname (__file__)
		icondir = path.join (dir, "..", "icons")
		icon_theme.prepend_search_path (icondir)

		self._language_bar = LanguageBar ()
		self._language_bar.connect ("property-activate",
						lambda widget, prop_name, prop_state: self._proxy.PropertyActivate (prop_name, prop_state))
		self._language_bar.connect ("get-im-menu",
						self._get_im_menu_cb)
		self._language_bar.focus_out ()
		self._language_bar.show_all ()

		self._candidate_panel = CandidatePanel ()
		self._candidate_panel.connect ("cursor-up",
						lambda widget: self._proxy.CursorUp ())
		self._candidate_panel.connect ("cursor-down",
						lambda widget: self._proxy.CursorDown ())

		self._status_icon = gtk.StatusIcon ()
		self._status_icon.connect ("popup-menu", self._status_icon_popup_menu_cb)
		self._status_icon.connect ("activate", self._status_icon_activate_cb)
		self._status_icon.set_from_icon_name ("engine-default")
		self._status_icon.set_tooltip ("iBus - Running")
		self._status_icon.set_visible (True)

	def set_cursor_location (self, x, y, w, h):
		self._candidate_panel.move (x + w, y + h)

	def update_preedit (self, text, attrs, cursor_pos, show):
		self._candidate_panel.update_preedit (text, attrs, cursor_pos, show)

	def show_preedit_string (self):
		self._candidate_panel.show_preedit_string ()

	def hide_preedit_string (self):
		self._candidate_panel.hide_preedit_string ()

	def update_aux_string (self, text, attrs, show):
		self._candidate_panel.update_aux_string (text, attrs, show)

	def show_aux_string (self):
		self._candidate_panel.show_aux_string ()

	def hide_aux_string (self):
		self._candidate_panel.hide_aux_string ()

	def update_lookup_table (self, lookup_table, show):
		self._candidate_panel.update_lookup_table (lookup_table, show)

	def show_candidate_window (self):
		self._candidate_panel.show ()

	def hide_candidate_window (self):
		self._candidate_panel.hide ()

	def show_language_bar (self):
		self._language_bar.show ()

	def hide_language_bar (self):
		self._language_bar.hide ()

	def register_properties (self, props):
		self._language_bar.register_properties (props)

	def update_property (self, prop):
		self._language_bar.update_property (prop)

	def _set_im_icon (self, icon_name):
		self._language_bar.set_im_icon (icon_name)
		self._status_icon.set_from_icon_name (icon_name)

	def focus_in (self, ic):
		self.reset ()
		self._focus_ic = ic

		factory, enabled = self._ibus.GetInputContextStates (ic)

		if factory == "" or not enabled:
			self._set_im_icon ("engine-default")
		else:
			name, lang, icon, authors, credits = self._ibus.GetFactoryInfo (factory)
			self._set_im_icon (icon)
		self._language_bar.focus_in ()

	def focus_out (self, ic):
		self.reset ()
		if self._focus_ic == ic:
			self._focus_ic = None
			self._language_bar.focus_out ()
			self._set_im_icon ("engine-default")

	def states_changed (self):
		if not self._focus_ic:
			return
		factory, enabled = self._ibus.GetInputContextStates (self._focus_ic)
		if not enabled:
			self._set_im_icon ("engine-default")
		else:
			name, lang, icon, authors, credits = self._ibus.GetFactoryInfo (factory)
			self._set_im_icon (icon)

	def reset (self):
		self._candidate_panel.reset ()
		self._language_bar.reset ()

	def do_destroy (self):
		gtk.main_quit ()

	def _create_im_menu (self):
		menu = gtk.Menu ()
		factories = self._ibus.GetFactories ()
		if not factories:
			item = gtk.MenuItem (label = "no engine")
			item.set_sensitive (False)
			menu.add (item)
		else:
			for factory in factories:
				name, lang, icon, authors, credits = self._ibus.GetFactoryInfo (factory)
				item = gtk.ImageMenuItem ("%s - %s" % (LANGUAGES.get (lang, lang), name))
				if not icon:
					icon = "engine-default"
				item.set_image (gtk.image_new_from_icon_name (icon, gtk.ICON_SIZE_MENU))
				item.connect ("activate", self._menu_item_activate_cb, factory)
				menu.add (item)

		menu.show_all ()
		menu.set_take_focus (False)
		return menu

	def _get_im_menu_cb (self, languagebar):
		menu = self._create_im_menu ()
		return menu

	def _status_icon_activate_cb (self, status_icon):
		if not self._focus_ic:
			return
		menu = self._create_im_menu ()
		menu.popup (None, None,
				gtk.status_icon_position_menu,
				0,
				gtk.get_current_event_time (),
				self._status_icon)

	def _status_icon_popup_menu_cb (self, status_icon, button, active_time):
		if not self._focus_ic:
			return
		menu = self._create_im_menu ()
		menu.popup (None, None,
				gtk.status_icon_position_menu,
				button,
				active_time,
				self._status_icon)

	def _menu_item_activate_cb (self, item, factory):
		self._ibus.SetFactory (factory)

gobject.type_register (Panel, "IBusPanel")

class PanelProxy (interface.IPanel):
	def __init__ (self, dbusconn, object_path, _ibus):
		interface.IPanel.__init__ (self, dbusconn, object_path)
		self._dbusconn = dbusconn
		self._panel = Panel (self, _ibus)

	def SetCursorLocation (self, x, y, w, h):
		self._panel.set_cursor_location (x, y, w, h)

	def UpdatePreedit (self, text, attrs, cursor_pos, show):
		attrs = ibus.attr_list_from_dbus_value (attrs)
		self._panel.update_preedit (text, atrrs, cursor_pos, show)

	def ShowPreeditString (self):
		self._panel.show_preedit_string ()

	def HidePreeditString (self):
		self._panel.hide_preedit_string ()

	def UpdateAuxString (self, text, attrs, show):
		attrs = ibus.attr_list_from_dbus_value (attrs)
		self._panel.update_aux_string (text, attrs, show)

	def ShowAuxString (self):
		self._panel.show_aux_string ()

	def HideAuxString (self):
		self._panel.hide_aux_string ()

	def UpdateLookupTable (self, lookup_table, show):
		lookup_table = ibus.lookup_table_from_dbus_value (lookup_table)
		self._panel.update_lookup_table (lookup_table, show)

	def ShowCandidateWindow (self):
		self._panel.show_candidate_window ()

	def HideCandidateWindow (self):
		self._panel.hide_candidate_window ()

	def ShowLanguageBar (self):
		self._panel.show_language_bar ()

	def HideLanguageBar (self):
		self._panel.hide_language_bar ()

	def RegisterProperties (self, props):
		props = ibus.prop_list_from_dbus_value (props)
		self._panel.register_properties (props)

	def UpdateProperty (self, prop):
		prop = ibus.property_from_dbus_value (prop)
		self._panel.update_property (prop)

	def FocusIn (self, ic):
		self._panel.focus_in (ic)

	def FocusOut (self, ic):
		self._panel.focus_out (ic)

	def StatesChanged (self):
		self._panel.states_changed ()

	def Reset (self):
		self._panel.reset ()

	def Destroy (self):
		self._panel.destroy ()

