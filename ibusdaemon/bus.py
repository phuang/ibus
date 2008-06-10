#!/usr/bin/env python
import weakref
import dbus
import ibus
from ibus import keysyms
from clientmanager import ClientManager
from factorymanager import FactoryManager
from connection import Connection
from panel import Panel, DummyPanel

class IBus (ibus.Object):
	def __init__ (self):
		ibus.Object.__init__ (self)
		self._connections = {}
		self._client_manager = ClientManager ()
		self._factory_manager = FactoryManager ()
		self._panel = DummyPanel ()

		self._focused_client = None
		self._last_focused_client = None
		self._client_handlers = []

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
	# methods for im client
	##########################################################
	def register_client (self, name, dbusconn):
		ibusconn = self._lookup_ibus_connection (dbusconn)
		client = self._client_manager.register_client (name, ibusconn)
		client.connect ("destroy", self._client_destroy_cb)
		factory = self._factory_manager.get_default_factory ()
		if factory:
			engine = factory.create_engine ()
			client.set_engine (engine)

	def focus_in (self, dbusconn):
		client = self._lookup_client (dbusconn)

		if self._focused_client != client and self._focused_client != None:
			map (self._focused_client.disconnect, self._client_handlers)
			del self._client_handlers[:]
			self._focused_client.focus_out ()

		# Install all callback functions
		id = client.connect ("update-preedit", self._update_preedit_cb)
		self._client_handlers.append (id)
		id = client.connect ("update-aux-string", self._update_aux_string_cb)
		self._client_handlers.append (id)
		id = client.connect ("update-lookup-table", self._update_lookup_table_cb)
		self._client_handlers.append (id)
		id = client.connect ("register-properties", self._register_properties_cb)
		self._client_handlers.append (id)
		id = client.connect ("update-property", self._update_property_cb)
		self._client_handlers.append (id)

		self._panel.reset ()
		self._focused_client = client
		self._last_focused_client = client
		client.focus_in ()

	def focus_out (self, dbusconn):
		client = self._lookup_client (dbusconn)
		if client == self._focused_client:
			map (self._focused_client.disconnect, self._client_handlers)
			del self._client_handlers[:]
			self._focused_client = None
		client.focus_out ()
		self._panel.reset ()

	def reset (self, dbusconn):
		client = self._lookup_client (dbusconn)
		self._panel.reset ()
		client.reset ()

	def is_enabled (self, dbusconn):
		client = self._lookup_client (dbusconn)
		return client.is_enabled ()

	def process_key_event (self, keyval, is_press, state, 
								dbusconn, reply_cb, error_cb):
		client = self._lookup_client (dbusconn)

		if self._filter_hotkeys (client, keyval, is_press, state):
			reply_cb (True)
			return
		else:
			client.process_key_event (keyval, is_press, state, reply_cb, error_cb)

	def set_cursor_location (self, x, y, w, h, dbusconn):
		client = self._lookup_client (dbusconn)
		client.set_cursor_location (x, y, w, h)
		self._panel.set_cursor_location (x, y, w, h)

	def _filter_hotkeys (self, client, keyval, is_press, state):
		if is_press and keyval == keysyms.space \
			and state == keysyms.CONTROL_MASK:
			enable = not client.is_enabled ()
			client.set_enable (enable)
			if client.get_engine () == None and enable:
				factory = self._factory_manager.get_default_factory ()
				if factory:
					engine = factory.create_engine ()
					client.set_engine (engine)
			return True
		return False

	def _lookup_client (self, dbusconn):
		ibusconn = self._lookup_ibus_connection (dbusconn)
		return self._client_manager.lookup_client (ibusconn)
		if dbusconn not in self._clients:
			raise ibus.IBusException ("not register the client")
		return self._clients[dbusconn]

	def _update_preedit_cb (self, client, text, attrs, cursor_pos, visible):
		assert self._focused_client == client

		self._panel.update_preedit_string (text, attrs, cursor_pos, visible)

	def _update_aux_string_cb (self, client, text, attrs, visible):
		assert self._focused_client == client

		self._panel.update_aux_string (text, attrs, visible)

	def _update_lookup_table_cb (self, client, lookup_table, visible):
		assert self._focused_client == client

		self._panel.update_lookup_table (lookup_table, visible)

	def _register_properties_cb (self, client, props):
		assert self._focused_client == client

		self._panel.register_properties (props)


	def _update_property_cb (self, client, prop):
		assert self._focused_client == client

		self._panel.update_property (prop)

	def _client_destroy_cb (self, client):
		if client == self._focused_client:
			del self._client_handlers[:]
			self._focused_client = None

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
		self._panel.connect ("destroy", self._panel_destroy_cb)

	def _panel_page_up_cb (self, panel):
		assert panel == self._panel
		if self._focused_client:
			self._focused_client.page_up ()

	def _panel_page_down_cb (self, panel):
		assert panel == self._panel
		if self._focused_client:
			self._focused_client.page_down ()

	def _panel_cursor_up_cb (self, panel):
		assert panel == self._panel
		if self._focused_client:
			self._focused_client.cursor_up ()

	def _panel_cursor_down_cb (self, panel):
		assert panel == self._panel
		if self._focused_client:
			self._focused_client.cursor_down ()

	def _panel_destroy_cb (self, panel):
		if panel == self._panel:
			self._panel = DummyPanel ()

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

	def RegisterClient (self, client_name, dbusconn):
		self._ibus.register_client (client_name, dbusconn)

	def RegisterFactories (self, object_paths, dbusconn):
		self._ibus.register_factories (object_paths, dbusconn)

	def UnregisterEngines (self, object_paths, dbusconn):
		self._ibus.unregister_engines (object_paths, dbusconn)

	def RegisterPanel (self, object_path, replace, dbusconn):
		self._ibus.register_panel (object_path, replace, dbusconn)

	def ProcessKeyEvent (self, keyval, is_press, state, \
							dbusconn, reply_cb, error_cb):
		try:
			self._ibus.process_key_event (keyval, is_press, state, 
							dbusconn, reply_cb, error_cb)
		except Exception, e:
			error_cb (e)

	def SetCursorLocation (self, x, y, w, h, dbusconn):
		self._ibus.set_cursor_location (x, y, w, h, dbusconn)

	def FocusIn (self, dbusconn):
		self._ibus.focus_in (dbusconn)

	def FocusOut (self, dbusconn):
		self._ibus.focus_out (dbusconn)

	def Reset (self, dbusconn):
		self._ibus.reset (dbusconn)

	def IsEnabled (self, dbusconn):
		return self._ibus.is_enabled (dbusconn)

