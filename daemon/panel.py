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

import weakref
import gobject
import ibus

class Panel(ibus.Object):
	__gsignals__ = {
		"page-up" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"page-down" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"cursor-up" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"cursor-down" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, gobject.TYPE_INT)),
		"property-show" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, )),
		"property-hide" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, )),
	}

	def __init__(self, ibusconn, object_path):
		super(Panel, self).__init__()
		self._ibusconn = ibusconn
		self._object_path = object_path
		self._panel = self._ibusconn.get_object(self._object_path)

		self._ibusconn.connect("destroy", self._ibusconn_destroy_cb)
		self._ibusconn.connect("dbus-signal", self._dbus_signal_cb)

	def set_cursor_location(self, x, y, w, h):
		self._panel.SetCursorLocation(x, y, w, h,
				**ibus.DEFAULT_ASYNC_HANDLERS)

	def update_preedit(self, text, attrs, cursor_pos, visible):
		self._panel.UpdatePreedit(text, attrs, cursor_pos, visible,
				**ibus.DEFAULT_ASYNC_HANDLERS)

	def update_aux_string(self, text, attrs, visible):
		self._panel.UpdateAuxString(text, attrs, visible,
				**ibus.DEFAULT_ASYNC_HANDLERS)

	def update_lookup_table(self, lookup_table, visible):
		self._panel.UpdateLookupTable(lookup_table, visible,
				**ibus.DEFAULT_ASYNC_HANDLERS)

	def register_properties(self, props):
		self._panel.RegisterProperties(props,
				**ibus.DEFAULT_ASYNC_HANDLERS)

	def update_property(self, prop):
		self._panel.UpdateProperty(prop,
				**ibus.DEFAULT_ASYNC_HANDLERS)

	def show_language_bar(self):
		self._panel.ShowLanguageBar(**ibus.DEFAULT_ASYNC_HANDLERS)

	def hide_language_bar(self):
		self._panel.HideLanguageBar(**ibus.DEFAULT_ASYNC_HANDLERS)

	def focus_in(self, ic):
		self._panel.FocusIn(ic, **ibus.DEFAULT_ASYNC_HANDLERS)

	def focus_out(self, ic):
		self._panel.FocusOut(ic, **ibus.DEFAULT_ASYNC_HANDLERS)

	def states_changed(self):
		self._panel.StatesChanged(**ibus.DEFAULT_ASYNC_HANDLERS)

	def reset(self):
		self._panel.Reset(**ibus.DEFAULT_ASYNC_HANDLERS)

	def destroy(self):
		if self._ibusconn != None:
			self._panel.Destroy(**ibus.DEFAULT_ASYNC_HANDLERS)

		self._ibusconn = None
		self._panel = None
		ibus.Object.destroy(self)

	# signal callbacks
	def _ibusconn_destroy_cb(self, ibusconn):
		self._ibusconn = None
		self.destroy()

	def _dbus_signal_cb(self, ibusconn, message):
		if message.is_signal(ibus.IBUS_PANEL_IFACE, "PageUp"):
			self.emit("page-up")
		elif message.is_signal(ibus.IBUS_PANEL_IFACE, "PageDown"):
			self.emit("page-down")
		elif message.is_signal(ibus.IBUS_PANEL_IFACE, "CursorUp"):
			self.emit("cursor-up")
		elif message.is_signal(ibus.IBUS_PANEL_IFACE, "CursorDown"):
			self.emit("cursor-down")
		elif message.is_signal(ibus.IBUS_PANEL_IFACE, "PropertyActivate"):
			args = message.get_args_list()
			self.emit("property-activate", args[0], args[1])
		elif message.is_signal(ibus.IBUS_PANEL_IFACE, "PropertyShow"):
			args = message.get_args_list()
			self.emit("property-show", args[0])
		elif message.is_signal(ibus.IBUS_PANEL_IFACE, "PropertyHide"):
			args = message.get_args_list()
			self.emit("property-hide", args[0])
		else:
			return False
		return True

	# methods for cmp
	# def __lt__(self, other):
	#		x = self.get_info()
	#		y = other.get_info()
	#		if x[1] < y[1]: return True
	#		if x[1] == y[1]: return x[0] < y[0]
	#
	#	def __gt__(self, other):
	#		x = self.get_info()
	#		y = other.get_info()
	#		if x[1] > y[1]: return True
	#		if x[1] == y[1]: return x[0] > y[0]

gobject.type_register(Panel)

class DummyPanel(ibus.Object):
	__gsignals__ = {
		"page-up" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"page-down" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"cursor-up" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"cursor-down" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, )),
	}

	def set_cursor_location(self, x, y, w, h):
		pass

	def update_preedit(self, text, attrs, cursor_pos, visible):
		pass

	def update_aux_string(self, text, attrs, visible):
		pass

	def update_lookup_table(self, lookup_table, visible):
		pass

	def register_properties(self, props):
		pass

	def update_property(self, prop):
		pass

	def show_language_bar(self):
		pass

	def hide_language_bar(self):
		pass

	def focus_in(self, ic):
		pass

	def focus_out(self, ic):
		pass

	def states_changed(self):
		pass

	def reset(self):
		pass

gobject.type_register(DummyPanel)
