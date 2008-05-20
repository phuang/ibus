import ibus
import dbus

class Application:
	def __init__ (self):
		self._dbusconn = dbus.connection.Connection (ibus.IBUS_ADDR)
		self._dbusconn.add_signal_receiver (self._disconnected_cb,
							"Disconnected",
							dbus_interface = dbus.LOCAL_IFACE)

	def _disconnected_cb (self):
		self.on_disconnected ()

	def on_disconnected (self):
		pass

	def run (self):
		pass

