#!/usr/bin/env python
import sys
import gobject
import dbus.server
import dbus.lowlevel
import dbus.mainloop.glib
import ibus
from bus import IBusProxy

class IBusServer (dbus.server.Server):
	def __init__ (self):
		dbus.server.Server.__init__ (self, ibus.IBUS_ADDR)
		self._ibus = IBusProxy ()

		self.register_object (self._ibus, ibus.IBUS_PATH)

	def new_connection (self, server, dbusconn):
		dbusconn.add_message_filter (self.message_filter_cb)
		self._ibus.new_connection (dbusconn)

	def remove_connection (self, dbusconn):
		self._ibus.remove_connection (dbusconn)

	def message_filter_cb (self, dbusconn, message):
		if message.is_method_call (dbus.LOCAL_IFACE, "Disconnected"):
			return dbus.lowlevel.HANDLER_RESULT_NOT_YET_HANDLED

		if message.get_type () == 4: # is signal
			if self._ibus.dispatch_dbus_signal (dbusconn, message):
				return dbus.lowlevel.HANDLER_RESULT_HANDLED

		return dbus.lowlevel.HANDLER_RESULT_NOT_YET_HANDLED

	def _print_message (self, message):
		print "Got a Message (%s) : " % message.__class__.__name__
		print "\t From:      %s" % message.get_sender ()
		print "\t To:        %s" % message.get_destination ()
		print "\t Interface: %s" % message.get_interface ()
		print "\t Path:      %s" % message.get_path ()
		print "\t Member:    %s" % message.get_member ()
		print "\t Arguments:"
		i = 0
		for arg in message.get_args_list():
			print "\t\t Arg[%d] : %s" % (i, arg)
			i = i + 1

if __name__ == "__main__":
	dbus.mainloop.glib.DBusGMainLoop (set_as_default = True)
	loop = gobject.MainLoop ()
	bus = IBusServer ()
	print "IBUS_ADDRESS=\"%s\"" % bus.get_address ()
	try:
		loop.run ()
	except KeyboardInterrupt, e:
		print "daemon exits"
		sys.exit ()

