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

