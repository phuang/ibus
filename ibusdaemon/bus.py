#!/usr/bin/env python
import weakref
import dbus
import ibus
from ibus import keysyms
from contextmanager import ContextManager
from factorymanager import FactoryManager
from connection import Connection
from panel import Panel, DummyPanel

class IBus (ibus.Object):
	def __init__ (self):
		ibus.Object.__init__ (self)
		self._connections = {}
		self._context_manager = ContextManager ()
		self._factory_manager = FactoryManager ()
		self._panel = DummyPanel ()

		self._focused_context = None
		self._last_focused_context = None
		self._context_handlers = []

		self._last_key = None

	def new_connection (self, dbusconn):
		assert dbusconn not in self._connections
		self._connections[dbusconn] = Connection (dbusconn)

	def remove_connection (self, dbusconn):
		assert dbusconn in self._connections

		# destroy the connection
		self._connections[dbusconn].destroy ()
		del self._connections[dbusconn]

	def _lookup_ibus_connection (self, dbusconn):
		if dbusconn not in self._connections:
			raise ibus.IBusException ("can not find ibus.Connection")
		return self._connections[dbusconn]

	##########################################################
	# methods for im context
	##########################################################
	def create_input_context (self, name, dbusconn):
		ibusconn = self._lookup_ibus_connection (dbusconn)
		context = self._context_manager.create_input_context (name, ibusconn)
		factory = self._factory_manager.get_default_factory ()
		if factory:
			engine = factory.create_engine ()
			context.set_engine (engine)
		return context.get_id ()

	def release_input_context (self, ic, dbusconn):
		ibusconn = self._lookup_ibus_connection (dbusconn)
		self._context_manager.release_input_context (ic, ibusconn)

	def focus_in (self, ic, dbusconn):
		context = self._lookup_context (ic, dbusconn)

		if self._focused_context != context and self._focused_context != None:
			map (self._focused_context.disconnect, self._context_handlers)
			self._context_handlers = []
			self._focused_context.focus_out ()

		# Install all callback functions
		id = context.connect ("update-preedit", self._update_preedit_cb)
		self._context_handlers.append (id)
		id = context.connect ("update-aux-string", self._update_aux_string_cb)
		self._context_handlers.append (id)
		id = context.connect ("update-lookup-table", self._update_lookup_table_cb)
		self._context_handlers.append (id)
		id = context.connect ("register-properties", self._register_properties_cb)
		self._context_handlers.append (id)
		id = context.connect ("update-property", self._update_property_cb)
		self._context_handlers.append (id)
		id = context.connect ("engine-lost", self._engine_lost_cb)
		self._context_handlers.append (id)
		id = context.connect ("destroy", self._context_destroy_cb)
		self._context_handlers.append (id)

		self._panel.reset ()
		self._focused_context = context
		self._last_focused_context = context
		context.focus_in ()

	def focus_out (self, ic, dbusconn):
		context = self._lookup_context (ic, dbusconn)
		if context == self._focused_context:
			map (self._focused_context.disconnect, self._context_handlers)
			self._context_handlers = []
			self._focused_context = None
		context.focus_out ()
		self._panel.reset ()

	def reset (self, ic, dbusconn):
		context = self._lookup_context (ic, dbusconn)
		context.reset ()

	def is_enabled (self, ic, dbusconn):
		context = self._lookup_context (ic, dbusconn)
		return context.is_enabled ()

	def process_key_event (self, ic, keyval, is_press, state,
								dbusconn, reply_cb, error_cb):
		context = self._lookup_context (ic, dbusconn)

		if self._filter_hotkeys (context, keyval, is_press, state):
			reply_cb (True)
			return
		else:
			context.process_key_event (keyval, is_press, state, reply_cb, error_cb)

	def set_cursor_location (self, ic, x, y, w, h, dbusconn):
		context = self._lookup_context (ic, dbusconn)
		context.set_cursor_location (x, y, w, h)
		self._panel.set_cursor_location (x, y, w, h)

	def _filter_hotkeys (self, context, keyval, is_press, state):
		if is_press and keyval == keysyms.space \
			and state == keysyms.CONTROL_MASK:
			enable = not context.is_enabled ()
			context.set_enable (enable)
			if context.get_engine () == None and enable:
				factory = self._factory_manager.get_default_factory ()
				if factory:
					engine = factory.create_engine ()
					context.set_engine (engine)
			return True
		return False

	def _lookup_context (self, ic, dbusconn):
		ibusconn = self._lookup_ibus_connection (dbusconn)
		return self._context_manager.lookup_context (ic, ibusconn)

	def _update_preedit_cb (self, context, text, attrs, cursor_pos, visible):
		assert self._focused_context == context

		self._panel.update_preedit_string (text, attrs, cursor_pos, visible)

	def _update_aux_string_cb (self, context, text, attrs, visible):
		assert self._focused_context == context

		self._panel.update_aux_string (text, attrs, visible)

	def _update_lookup_table_cb (self, context, lookup_table, visible):
		assert self._focused_context == context

		self._panel.update_lookup_table (lookup_table, visible)

	def _register_properties_cb (self, context, props):
		assert self._focused_context == context

		self._panel.register_properties (props)


	def _update_property_cb (self, context, prop):
		assert self._focused_context == context

		self._panel.update_property (prop)

	def _engine_lost_cb (self, context):
		assert self._focused_context == context

		self._panel.reset ()

	def _context_destroy_cb (self, context):
		assert context == self._focused_context
		self._context_handlers = []
		self._focused_context = None

	##########################################################
	# methods for im engines
	##########################################################
	def register_factories (self, object_paths, dbusconn):
		ibusconn = self._lookup_ibus_connection (dbusconn)
		self._factory_manager.register_factories (object_paths, ibusconn)

	def dispatch_dbus_signal (self, dbusconn, message):
		ibusconn = self._lookup_ibus_connection (dbusconn)
		ibusconn.dispatch_dbus_signal (message)

	def _lookup_engine (self, dbusconn, path):
		ibusconn = self._lookup_ibus_connection (dbusconn)
		return self._factory_manager.lookup_engine (ibusconn, path)


	##########################################################
	# methods for panel
	##########################################################
	def register_panel (self, object_path, replace, dbusconn):
		if not isinstance (self._panel, DummyPanel) and replace == False:
			raise ibus.Exception ("has have a panel!")
		if not isinstance (self._panel, DummyPanel):
			self._panel.destroy ()
		ibusconn = self._lookup_ibus_connection (dbusconn)
		self._panel = Panel (ibusconn, object_path)
		self._panel.connect ("page-up", self._panel_page_up_cb)
		self._panel.connect ("page-down", self._panel_page_down_cb)
		self._panel.connect ("cursor-up", self._panel_cursor_up_cb)
		self._panel.connect ("cursor-down", self._panel_cursor_down_cb)
		self._panel.connect ("property-activate", self._panel_property_active_cb)
		self._panel.connect ("destroy", self._panel_destroy_cb)

	def _panel_page_up_cb (self, panel):
		assert panel == self._panel
		if self._focused_context:
			self._focused_context.page_up ()

	def _panel_page_down_cb (self, panel):
		assert panel == self._panel
		if self._focused_context:
			self._focused_context.page_down ()

	def _panel_cursor_up_cb (self, panel):
		assert panel == self._panel
		if self._focused_context:
			self._focused_context.cursor_up ()

	def _panel_cursor_down_cb (self, panel):
		assert panel == self._panel
		if self._focused_context:
			self._focused_context.cursor_down ()

	def _panel_property_active_cb (self, panel, prop_name):
		assert panel == self._panel
		if self._focused_context:
			self._focused_context.property_activate (prop_name)

	def _panel_destroy_cb (self, panel):
		if panel == self._panel:
			self._panel = DummyPanel ()

	##########################################################
	# general methods
	##########################################################
	def get_factories (self):
		return self._factory_manager.get_factories ()

	def get_factory_info (self, factory_path):
		return self._factory_manager.get_factory_info (factory_path)

	def set_factory (self, factory_path):
		if self._focused_context == None:
			return
		self._panel.reset ()
		factory = self._factory_manager.get_factory (factory_path)
		engine = factory.create_engine ()
		self._focused_context.set_engine (engine)

class IBusProxy (ibus.IIBus):
	SUPPORTS_MULTIPLE_CONNECTIONS = True

	def __init__ (self):
		ibus.IIBus.__init__ (self)
		self._ibus = IBus ()

	def new_connection (self, dbusconn):
		self._ibus.new_connection (dbusconn)

	def remove_connection (self, dbusconn):
		self._ibus.remove_connection (dbusconn)

	def dispatch_dbus_signal (self, dbusconn, message):
		return self._ibus.dispatch_dbus_signal (dbusconn, message)

	def GetIBusAddress (self, dbusconn):
		return self._ibus_addr

	def CreateInputContext (self, context_name, dbusconn):
		return self._ibus.create_input_context (context_name, dbusconn)

	def ReleaseInputContext (self, ic, dbusconn):
		self._ibus.release_input_context (ic, dbusconn)

	def RegisterFactories (self, object_paths, dbusconn):
		self._ibus.register_factories (object_paths, dbusconn)

	def UnregisterEngines (self, object_paths, dbusconn):
		self._ibus.unregister_engines (object_paths, dbusconn)

	def RegisterPanel (self, object_path, replace, dbusconn):
		self._ibus.register_panel (object_path, replace, dbusconn)

	def ProcessKeyEvent (self, ic, keyval, is_press, state, \
							dbusconn, reply_cb, error_cb):
		try:
			self._ibus.process_key_event (ic, keyval, is_press, state,
							dbusconn, reply_cb, error_cb)
		except Exception, e:
			error_cb (e)

	def SetCursorLocation (self, ic, x, y, w, h, dbusconn):
		self._ibus.set_cursor_location (ic, x, y, w, h, dbusconn)

	def FocusIn (self, ic, dbusconn):
		self._ibus.focus_in (ic, dbusconn)

	def FocusOut (self, ic, dbusconn):
		self._ibus.focus_out (ic, dbusconn)

	def Reset (self, ic, dbusconn):
		self._ibus.reset (ic, dbusconn)

	def IsEnabled (self, ic, dbusconn):
		return self._ibus.is_enabled (ic, dbusconn)

	def GetFactories (self, dbusconn):
		return self._ibus.get_factories ()

	def GetFactoryInfo (self, factory_path, dbusconn):
		return self._ibus.get_factory_info (factory_path)

	def SetFactory (self, factory_path, dbusconn):
		return self._ibus.set_factory (factory_path)

