import dbus.service
from ibus.common import \
	IBUS_ENGINE_FACTORY_IFACE

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

