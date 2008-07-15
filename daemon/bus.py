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

import dbus
import ibus
from ibus import keysyms
from ibus import modifier
from contextmanager import ContextManager
from factorymanager import FactoryManager
from connection import Connection
from panel import Panel, DummyPanel
from config import Config, DummyConfig
from register import Register

class IBus(ibus.Object):
	def __init__(self):
		super(IBus, self).__init__()
		self._context_manager = ContextManager()
		self._factory_manager = FactoryManager()
		self._panel = DummyPanel()
		self._config = DummyConfig()
		self._register = Register()
		self._config_watch = dict()

		self._focused_context = None
		self._last_focused_context = None
		self._context_handlers = list()

		self._last_key = None

	##########################################################
	# methods for im context
	##########################################################
	def create_input_context(self, name, conn):
		context = self._context_manager.create_input_context(name, conn)
		factory = self._factory_manager.get_default_factory()
		if factory:
			engine = factory.create_engine()
			context.set_engine(engine)
		return context.get_id()

	def release_input_context(self, ic, conn):
		self._context_manager.release_input_context(ic, conn)

	def focus_in(self, ic, conn):
		context = self._lookup_context(ic, conn)

		if self._focused_context != context and self._focused_context != None:
			self._remove_focused_context_handlers()
			self._focused_context.focus_out()

		self._focused_context = context
		self._install_focused_context_handlers()

		self._panel.focus_in(context.get_id())
		self._last_focused_context = context
		context.focus_in()

	def focus_out(self, ic, conn):
		context = self._lookup_context(ic, conn)

		if context == self._focused_context:
			self._remove_focused_context_handlers()
			self._focused_context = None

		context.focus_out()
		self._panel.focus_out(context.get_id())

	def reset(self, ic, conn):
		context = self._lookup_context(ic, conn)
		context.reset()

	def is_enabled(self, ic, conn):
		context = self._lookup_context(ic, conn)
		return context.is_enabled()

	def process_key_event(self, ic, keyval, is_press, state,
								conn, reply_cb, error_cb):
		context = self._lookup_context(ic, conn)

		if self._filter_hotkeys(context, keyval, is_press, state):
			reply_cb(True)
			return
		else:
			context.process_key_event(keyval, is_press, state, reply_cb, error_cb)

	def set_cursor_location(self, ic, x, y, w, h, conn):
		context = self._lookup_context(ic, conn)
		context.set_cursor_location(x, y, w, h)
		self._panel.set_cursor_location(x, y, w, h)

	def _filter_hotkeys(self, context, keyval, is_press, state):
		if is_press and keyval == keysyms.space \
			and (state & ~modifier.MOD2_MASK) == modifier.CONTROL_MASK:
			enable = not context.is_enabled()
			if context.get_engine() == None and enable:
				factory = self._factory_manager.get_default_factory()
				if factory:
					engine = factory.create_engine()
					context.set_engine(engine)
			context.set_enable(enable)
			self._panel.states_changed()
			return True
		return False

	def _lookup_context(self, ic, conn):
		return self._context_manager.lookup_context(ic, conn)

	def _install_focused_context_handlers(self):
		# Install all callback functions
		if self._focused_context == None:
			return
		signals = (
			("update-preedit", self._update_preedit_cb),
			("update-aux-string", self._update_aux_string_cb),
			("update-lookup-table", self._update_lookup_table_cb),
			("register-properties", self._register_properties_cb),
			("update-property", self._update_property_cb),
			("engine-lost", self._engine_lost_cb),
			("destroy", self._context_destroy_cb)
		)
		for name, handler in signals:
			id = self._focused_context.connect(name, handler)
			self._context_handlers.append(id)

	def _remove_focused_context_handlers(self):
		if self._focused_context == None:
			return
		map(self._focused_context.disconnect, self._context_handlers)
		self._context_handlers = []

	def _update_preedit_cb(self, context, text, attrs, cursor_pos, visible):
		assert self._focused_context == context

		self._panel.update_preedit_string(text, attrs, cursor_pos, visible)

	def _update_aux_string_cb(self, context, text, attrs, visible):
		assert self._focused_context == context

		self._panel.update_aux_string(text, attrs, visible)

	def _update_lookup_table_cb(self, context, lookup_table, visible):
		assert self._focused_context == context

		self._panel.update_lookup_table(lookup_table, visible)

	def _register_properties_cb(self, context, props):
		assert self._focused_context == context

		self._panel.register_properties(props)


	def _update_property_cb(self, context, prop):
		assert self._focused_context == context

		self._panel.update_property(prop)

	def _engine_lost_cb(self, context):
		assert self._focused_context == context

		self._panel.reset()

	def _context_destroy_cb(self, context):
		assert context == self._focused_context
		self._remove_focused_context_handlers()
		self._focused_context = None
		self._panel.reset()

	##########################################################
	# methods for im engines
	##########################################################
	def register_factories(self, object_paths, conn):
		self._factory_manager.register_factories(object_paths, conn)

	def _lookup_engine(self, dbusconn, path):
		return self._factory_manager.lookup_engine(conn, path)


	##########################################################
	# methods for panel
	##########################################################
	def register_panel(self, object_path, replace, conn):
		if not isinstance(self._panel, DummyPanel) and replace == False:
			raise ibus.Exception("has have a panel!")
		if not isinstance(self._panel, DummyPanel):
			self._panel.destroy()
		self._panel = Panel(conn, object_path)
		self._panel.connect("page-up", self._panel_page_up_cb)
		self._panel.connect("page-down", self._panel_page_down_cb)
		self._panel.connect("cursor-up", self._panel_cursor_up_cb)
		self._panel.connect("cursor-down", self._panel_cursor_down_cb)
		self._panel.connect("property-activate", self._panel_property_active_cb)
		self._panel.connect("property-show", self._panel_property_show_cb)
		self._panel.connect("property-hide", self._panel_property_hide_cb)
		self._panel.connect("destroy", self._panel_destroy_cb)

	def _panel_page_up_cb(self, panel):
		assert panel == self._panel
		if self._focused_context:
			self._focused_context.page_up()

	def _panel_page_down_cb(self, panel):
		assert panel == self._panel
		if self._focused_context:
			self._focused_context.page_down()

	def _panel_cursor_up_cb(self, panel):
		assert panel == self._panel
		if self._focused_context:
			self._focused_context.cursor_up()

	def _panel_cursor_down_cb(self, panel):
		assert panel == self._panel
		if self._focused_context:
			self._focused_context.cursor_down()

	def _panel_property_active_cb(self, panel, prop_name, prop_state):
		assert panel == self._panel
		if self._focused_context:
			self._focused_context.property_activate(prop_name, prop_state)

	def _panel_property_show_cb(self, panel, prop_name):
		assert panel == self._panel
		if self._focused_context:
			self._focused_context.property_show(prop_name)

	def _panel_property_hide_cb(self, panel, prop_name):
		assert panel == self._panel
		if self._focused_context:
			self._focused_context.property_hide(prop_name)

	def _panel_destroy_cb(self, panel):
		if panel == self._panel:
			self._panel = DummyPanel()

	##########################################################
	# methods for panel
	##########################################################
	def register_config(self, object_path, replace, conn):
		if not isinstance(self._config, DummyConfig) and replace == False:
			raise ibus.Exception("has have a config!")
		if not isinstance(self._config, DummyConfig):
			self._config.destroy()
		self._config = Config(conn, object_path)
		self._config.connect("value-changed", self._config_value_changed_cb)
		self._config.connect("destroy", self._config_destroy_cb)

	def config_set_string(self, key, value, dbusconn, **kargs):
		self._config.set_string(key, value, **kargs)

	def config_set_int(self, key, value, dbusconn, **kargs):
		self._config.set_int(key, value, **kargs)

	def config_set_bool(self, key, value, dbusconn, **kargs):
		self._config.set_bool(key, value, **kargs)

	def config_get_string(self, key, dbusconn, **kargs):
		self._config.get_string(key, value, **kargs)

	def config_get_int(self, key, dbusconn, **kargs):
		self._config.get_int(key, value, **kargs)

	def config_get_bool(self, key, dbusconn, **kargs):
		self._config.get_bool(key, value, **kargs)

	def config_add_watch_dir(self, dir, conn, **kargs):
		if not dir.endswith("/"):
			dir += "/"

		if conn.add_watch_dir(dir):
			if dir not in self._config_watch:
				self._config_watch[dir] = set()
			self._config_watch[dir].add(conn)

	def config_remove_watch_dir(self, dir, conn, **kargs):
		if not dir.endswith("/"):
			dir += "/"

		if conn.remove_watch_dir(dir):
			if dir in self._config_watch:
				self._config_watch[dir].remove(conn)

	def _config_value_changed_cb(self, config, key, value):
		for dir in self._config_watch.keys():
			if dir.startswith(key):
				for conn in self._config[dir]:
					conn.emit_dbus_signal("ConfigValueChanged", key, value)

	def _config_destroy_cb(self, config, key, value):
		if config == self._config:
			self._config = DummyConfig()

	##########################################################
	# engine register methods
	##########################################################
	def register_list_engines(self, conn):
		return self._register.list_engines()

	def register_start_engine(self, lang, name, conn):
		return self._register.start_engine(lang, name)

	def register_restart_engine(self, lang, name, conn):
		return self._register.restart_engine(lang, name)

	def register_stop_engine(self, lang, name, conn):
		return self._register.stop_engine(lang, name)

	##########################################################
	# general methods
	##########################################################
	def get_factories(self):
		return self._factory_manager.get_factories()

	def get_factory_info(self, factory_path):
		return self._factory_manager.get_factory_info(factory_path)

	def set_factory(self, factory_path):
		if self._focused_context == None:
			return
		self._panel.reset()
		factory = self._factory_manager.get_factory(factory_path)
		engine = factory.create_engine()
		self._focused_context.set_engine(engine)
		self._focused_context.set_enable(True)
		engine.focus_in()
		self._panel.states_changed()

	def get_input_context_states(self, ic, conn):
		factory_path = ""
		context = self._lookup_context(ic, conn)
		if context.get_factory() != None:
			factory_path = context.get_factory().get_object_path()
		return factory_path, context.is_enabled()


class IBusProxy(ibus.IIBus):
	def __init__(self, bus, dbusconn):
		super(IBusProxy, self).__init__(dbusconn, ibus.IBUS_PATH)
		self._ibus = bus
		self._conn = Connection(dbusconn)

	def new_connection(self, dbusconn):
		self._ibus.new_connection(dbusconn)

	def remove_connection(self, dbusconn):
		self._ibus.remove_connection(dbusconn)

	def dispatch_dbus_signal(self, dbusconn, message):
		return self._conn.dispatch_dbus_signal(dbusconn, message)

	def GetIBusAddress(self, dbusconn):
		return self._ibus_addr

	def CreateInputContext(self, context_name, dbusconn):
		return self._ibus.create_input_context(context_name, self._conn)

	def ReleaseInputContext(self, ic, dbusconn):
		self._ibus.release_input_context(ic, self._conn)

	def RegisterFactories(self, object_paths, dbusconn):
		self._ibus.register_factories(object_paths, self._conn)

	def UnregisterEngines(self, object_paths, dbusconn):
		self._ibus.unregister_engines(object_paths, self._conn)

	def RegisterPanel(self, object_path, replace, dbusconn):
		self._ibus.register_panel(object_path, replace, self._conn)

	def RegisterConfig(self, object_path, replace, dbusconn):
		self._ibus.register_config(object_path, replace, self._conn)

	def ProcessKeyEvent(self, ic, keyval, is_press, state, \
							dbusconn, reply_cb, error_cb):
		try:
			self._ibus.process_key_event(ic, keyval, is_press, state,
							self._conn, reply_cb, error_cb)
		except Exception, e:
			error_cb(e)

	def SetCursorLocation(self, ic, x, y, w, h, dbusconn):
		self._ibus.set_cursor_location(ic, x, y, w, h, self._conn)

	def FocusIn(self, ic, dbusconn):
		self._ibus.focus_in(ic, self._conn)

	def FocusOut(self, ic, dbusconn):
		self._ibus.focus_out(ic, self._conn)

	def Reset(self, ic, dbusconn):
		self._ibus.reset(ic, self._conn)

	def IsEnabled(self, ic, dbusconn):
		return self._ibus.is_enabled(ic, self._conn)

	def GetFactories(self, dbusconn):
		return self._ibus.get_factories()

	def GetFactoryInfo(self, factory_path, dbusconn):
		return self._ibus.get_factory_info(factory_path)

	def SetFactory(self, factory_path, dbusconn):
		return self._ibus.set_factory(factory_path)

	def GetInputContextStates(self, ic, dbusconn):
		return self._ibus.get_input_context_states(ic, self._conn)

	def ConfigSetString(self, key, value, dbusconn, reply_cb, error_cb):
		self._ibus.config_set_string(key, value, self._conn,
				reply_handler = reply_cb,
				error_handler = error_cb)

	def ConfigSetInt(self, key, value, dbusconn, reply_cb, error_cb):
		self._ibus.config_set_int(key, value, self._conn,
				reply_handler = reply_cb,
				error_handler = error_cb)

	def ConfigSetBool(self, key, value, dbusconn, reply_cb, error_cb):
		self._ibus.config_set_bool(key, value, self._conn,
				reply_handler = reply_cb,
				error_handler = error_cb)

	def ConfigGetString(self, key, dbusconn, reply_cb, error_cb):
		self._ibus.config_get_string(key, self._conn,
				reply_handler = reply_cb,
				error_handler = error_cb)

	def ConfigGetInt(self, key, dbusconn, reply_cb, error_cb):
		self._ibus.config_get_int(key, self._conn,
				reply_handler = reply_cb,
				error_handler = error_cb)

	def ConfigGetBool(self, key, dbusconn, reply_cb, error_cb):
		self._ibus.config_get_bool(key, self._conn,
				reply_handler = reply_cb,
				error_handler = error_cb)

	def RegisterListEngines(self, dbusconn):
		return self._ibus.register_list_engines(self._conn)

	def RegisterStartEngine(self, lang, name, dbusconn):
		return self._ibus.register_start_engine(lang, name, self._conn)

	def RegisterRestartEngine(self, lang, name, dbusconn):
		return self._ibus.register_restart_engine(lang, name, self._conn)

	def RegisterStopEngine(self, lang, name, dbusconn):
		return self._ibus.register_stop_engine(lang, name, self._conn)

