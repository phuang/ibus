import dbus.service
from common import \
	IBUS_IFACE, \
	IBUS_PANEL_IFACE, \
	IBUS_ENGINE_IFACE, \
	IBUS_ENGINE_FACTORY_IFACE

class IIBus (dbus.service.Object):
	# define method decorator.
	method = lambda **args: \
		dbus.service.method (dbus_interface = IBUS_IFACE, \
							 connection_keyword = "dbusconn", \
							 **args)

	# define async method decorator.
	async_method = lambda **args: \
		dbus.service.method (dbus_interface = IBUS_IFACE, \
							 connection_keyword = "dbusconn", \
							 async_callbacks = ("reply_cb", "error_cb"), \
							 **args)

	@method (out_signature = "s")
	def GetIBusAddress (self, dbusconn): pass

	@method (in_signature = "s")
	def RegisterClient (self, client_name, dbusconn): pass

	@method ()
	def UnregisterClient (self, dbusconn): pass

	@method (in_signature = "ao")
	def RegisterFactories (self, object_paths, dbusconn): pass

	@method (in_signature = "ao")
	def UnregisterFactories (self, object_paths, dbusconn): pass

	@method (in_signature = "ob")
	def RegisterPanel (self, object_path, replace, dbusconn): pass

	@async_method (in_signature = "ubu", out_signature = "b")
	def ProcessKeyEvent (self, keyval, is_press, state, dbusconn, reply_cb, error_cb): pass

	@method (in_signature = "iiii")
	def SetCursorLocation (self, x, y, w, h, dbusconn): pass

	@method ()
	def FocusIn (self, dbusconn): pass

	@method ()
	def FocusOut (self, dbusconn): pass

	@method ()
	def Reset (self, dbusconn): pass

	@method (out_signature = "b")
	def IsEnabled (self, dbusconn): pass

class IEngineFactory (dbus.service.Object):
	# define method decorator.
	method = lambda **args: \
		dbus.service.method (dbus_interface = IBUS_ENGINE_FACTORY_IFACE, \
							 **args)

	# define async method decorator.
	async_method = lambda **args: \
		dbus.service.method (dbus_interface = IBUS_ENGINE_FACTORY_IFACE, \
							 async_callbacks = ("reply_cb", "error_cb"), \
							 **args)
	# Return a array. [name, language, icon_path, authors, credits]
	@method (out_signature = "as")
	def GetInfo (self): pass

	# Factory should allocate all resources in this method
	@method ()
	def Initialize (self): pass

	# Factory should free all allocated resources in this method
	@method ()
	def Uninitialize (self): pass

	# Create an input context and return the id of the context.
	# If failed, it will return "" or None.
	@method (out_signature = "o")
	def CreateEngine (self): pass

class IEngine (dbus.service.Object):
	# define method decorator.
	method = lambda **args: \
		dbus.service.method (dbus_interface = IBUS_ENGINE_IFACE, \
							 **args)

	# define signal decorator.
	signal = lambda **args: \
		dbus.service.signal (dbus_interface = IBUS_ENGINE_IFACE, \
							 **args)

	# define async method decorator.
	async_method = lambda **args: \
		dbus.service.method (dbus_interface = IBUS_ENGINE_IFACE, \
							 async_callbacks = ("reply_cb", "error_cb"), \
							 **args)

	@method (in_signature = "ubu", out_signature = "b")
	def ProcessKeyEvent (self, keyval, is_press, state):
		pass

	@method (in_signature = "iiii")
	def SetCursorLocation (self, x, y, w, h): pass

	@method ()
	def FocusIn (self): pass

	@method ()
	def FocusOut (self): pass

	@method ()
	def Reset (self): pass

	@method (in_signature = "b")
	def SetEnable (self, enable): pass

	@method (in_signature = "s")
	def TriggerProperty (self, property): pass

	@method ()
	def Destroy (self): pass

	@signal (signature="s")
	def CommitString (self, text): pass

	@signal (signature="ubu")
	def ForwardKeyEvent (self, keyval, is_press, state): pass

	@signal (signature="svu")
	def PreeditChanged (self, text, attrs, cursor_pos): pass

	# below signals are optional. The engine could create and maintain panel by self.
	@signal (signature="v")
	def UpdateProperties (self, properties): pass

	@signal (signature="sv")
	def AuxStringChanged (self, text, attrs): pass

	@signal (signature="v")
	def UpdateLookupTable (self, lookup_table): pass

	@signal ()
	def ShowLookupTable (self): pass

	@signal ()
	def HideLookupTable (self): pass

class IPanel (dbus.service.Object):
	# define method decorator.
	method = lambda **args: \
		dbus.service.method (dbus_interface = IBUS_PANEL_IFACE, \
							 **args)

	# define signal decorator.
	signal = lambda **args: \
		dbus.service.signal (dbus_interface = IBUS_PANEL_IFACE, \
							 **args)

	# define async method decorator.
	async_method = lambda **args: \
		dbus.service.method (dbus_interface = IBUS_PANE_IFACE, \
							 async_callbacks = ("reply_cb", "error_cb"), \
							 **args)
	@method (in_signature="svu")
	def SetPreeditString (self, text, attrs, cursor_pos): pass

	@method ()
	def ShowPreeditString (self): pass

	@method ()
	def HidePreeditString (self): pass

	@method (in_signature="sv")
	def SetAuxString (self, text, attrs): pass

	@method ()
	def ShowAuxString (self): pass

	@method ()
	def HideAuxString (self): pass

	@method (in_signature="v")
	def UpdateLookupTable (self, lookup_table): pass

	@method ()
	def ShowCandidateWindow (self): pass

	@method ()
	def HideCandidateWindow (self): pass

	@method ()
	def ShowLanguageBar (self): pass

	@method ()
	def HideLanguageBar (self): pass

	@method ()
	def Destroy (self): pass

	#signals
	@signal ()
	def PageUp (self): pass

	@signal ()
	def PageDown (self): pass

	@signal ()
	def CursorBack (self): pass

	@signal ()
	def CursorForward (self): pass

