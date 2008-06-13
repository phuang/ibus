import dbus
import dbus.connection
import dbus.mainloop.glib
import ibus
import factory
import gtk

class IMApp:
	def __init__ (self):
		self._dbusconn = dbus.connection.Connection (ibus.IBUS_ADDR)
		self._dbusconn.add_signal_receiver (self._disconnected_cb, 
							"Disconnected", 
							dbus_interface = dbus.LOCAL_IFACE)
		self._engine = factory.DemoEngineFactory (self._dbusconn)
		self._ibus = self._dbusconn.get_object (ibus.IBUS_NAME, ibus.IBUS_PATH)
		self._ibus.RegisterFactories ([factory.FACTORY_PATH], **ibus.DEFAULT_ASYNC_HANDLERS)

	def run (self):
		gtk.main ()

	def _disconnected_cb (self):
		print "disconnected"
		gtk.main_quit ()
	

def main ():
	IMApp ().run ()

if __name__ == "__main__":
	dbus.mainloop.glib.DBusGMainLoop (set_as_default=True)
	main ()
