import dbus.service
from ibus.common import \
	IBUS_PANEL_IFACE

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
	@method (in_signature="iiii")
	def SetCursorLocation (self, x, y, w, h): pass

	@method (in_signature="svub")
	def UpdatePreedit (self, text, attrs, cursor_pos, visible): pass

	@method (in_signature="svb")
	def UpdateAuxString (self, text, attrs, visible): pass

	@method (in_signature="vb")
	def UpdateLookupTable (self, lookup_table, visible): pass

	@method (in_signature="v")
	def UpdateProperties (self, props): pass

	@method ()
	def ShowLanguageBar (self): pass

	@method ()
	def HideLanguageBar (self): pass

	@method ()
	def Reste (self): pass

	@method ()
	def Destroy (self): pass

	#signals
	@signal ()
	def PageUp (self): pass

	@signal ()
	def PageDown (self): pass

	@signal ()
	def CursorUp (self): pass

	@signal ()
	def CursorDown (self): pass

