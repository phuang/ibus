import dbus.service
from ibus.common import \
	IBUS_IFACE

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

	# methods for ibus clients
	@method (in_signature = "s", out_signature = "s")
	def CreateInputContext (self, client_name, dbusconn): pass

	@method (in_signature = "s")
	def ReleaseInputContext (self, ic, dbusconn): pass

	@async_method (in_signature = "subu", out_signature = "b")
	def ProcessKeyEvent (self, ic, keyval, is_press, state, dbusconn, reply_cb, error_cb): pass

	@method (in_signature = "siiii")
	def SetCursorLocation (self, ic, x, y, w, h, dbusconn): pass

	@method (in_signature = "s")
	def FocusIn (self, ic, dbusconn): pass

	@method (in_signature = "s")
	def FocusOut (self, ic, dbusconn): pass

	@method (in_signature = "s")
	def Reset (self, ic, dbusconn): pass

	@method (in_signature = "s", out_signature = "b")
	def IsEnabled (self, ic, dbusconn): pass

	# methods for ibus engine provide
	@method (in_signature = "ao")
	def RegisterFactories (self, object_paths, dbusconn): pass

	@method (in_signature = "ao")
	def UnregisterFactories (self, object_paths, dbusconn): pass

	# methods for ibus panel
	@method (in_signature = "ob")
	def RegisterPanel (self, object_path, replace, dbusconn): pass

	# general methods
	@method (out_signature = "av")
	def GetFactories (self, dbusconn): pass

	@method (in_signature = "o", out_signature = "av")
	def GetFactoryInfo (self, factory_path, dbusconn): pass

	@method (in_signature = "o")
	def SetFactory (self, factory_path, dbusconn): pass

	#sigals
	def CommitString (self, ic, text): pass

	def UpdatePreedit (self, ic, text, attrs, cursor_pos, show): pass

	def Enabled (self, ic): pass

	def Disabled (self, ic): pass
