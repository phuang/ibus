import dbus.lowlevel
import ibus
import gobject

class Connection (ibus.Object):
	__gsignals__ = {
		"dbus-signal" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_PYOBJECT, )
		)
	}
	def __init__ (self, dbusconn):
		ibus.Object.__init__ (self)
		self._dbusconn = dbusconn

	def get_object (self, path):
		return self._dbusconn.get_object ("no.name", path)

	def emit_dbus_signal (self, name, *args):
		message = dbus.lowlevel.SignalMessage (ibus.IBUS_PATH, ibus.IBUS_IFACE, name)
		message.append (*args)
		self._dbusconn.send_message (message)
		self._dbusconn.flush ()

	def do_destroy (self):
		self._dbusconn = None

	def dispatch_dbus_signal (self, message):
		self.emit ("dbus-signal", message)

gobject.type_register (Connection)
