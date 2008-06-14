# -*- coding: utf-8 -*-
import gtk
import gtk.gdk as gdk
import gobject
import ibus
from lang import LANGUAGES
from ibus import interface
from languagebar import LanguageBar
from candidatepanel import CandidatePanel

class Panel (ibus.Object):
	def __init__ (self, proxy, _ibus):
		gobject.GObject.__init__ (self)
		self._proxy = proxy
		self._ibus = _ibus
		self._language_bar = LanguageBar ()
		self._language_bar.connect ("property-activate",
						lambda widget, prop_name: self._proxy.PropertyActivate (prop_name))
		self._language_bar.connect ("im-menu-popup",
						self._im_menu_popup_cb)
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
		self._language_bar.update_property (self, prop)

	def reset (self):
		self._candidate_panel.reset ()
		self._language_bar.reset ()

	def do_destroy (self):
		gtk.main_quit ()

	def _create_im_menu (self):
		menu = gtk.Menu ()
		factories = self._ibus.GetFactories ()
		if not factories:
			item = gtk.MenuItem (label = "no im")
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

	def _menu_position_cb (self, menu, button):
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

	def _im_menu_popup_cb (self, languagebar, button):
		menu = self._create_im_menu ()
		menu.popup (None, None,
				self._menu_position_cb,
				0,
				gtk.get_current_event_time (),
				button)

	def _status_icon_activate_cb (self, status_icon):
		menu = self._create_im_menu ()
		menu.popup (None, None,
				gtk.status_icon_position_menu,
				0,
				gtk.get_current_event_time (),
				self._status_icon)

	def _status_icon_popup_menu_cb (self, status_icon, button, active_time):
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
		prop = ibus.property_from_dbus_value (props)
		self._panel.update_property (prop)

	def Reset (self):
		self._panel.reset ()

	def Destroy (self):
		self._panel.destroy ()

