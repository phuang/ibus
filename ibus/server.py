#!/usr/bin/env python
import dbus
import dbus.server
import dbus.lowlevel
import dbus.mainloop.glib
import gobject
from ibus import keysyms
from ibus import attribute
from ibus import client
from ibus import interface
from ibus import IBus
from ibus import Object

from ibus.common import \
		IBUS_PATH, \
		IBUS_ADDR

class IBusServer (Object):
	def __init__ (self):
		Object.__init__ (self)
		self._server = dbus.server.Server (IBUS_ADDR)
		self._ibus = IBus ()
		self._server.register_object (self._ibus, IBUS_PATH)

	def get_address (self):
		return self._server.get_address ()

	def new_connection (self, server, connection):
		connection.add_message_filter (self.message_filter_cb)
		pass

	def remove_connection (self, connection):
		pass

	def message_filter_cb (self, connection, message):
		if message.get_interface() == "org.freedesktop.DBus.Local" and \
			message.get_member() == "Disconnected":
			return dbus.lowlevel.HANDLER_RESULT_NOT_YET_HANDLED

		return dbus.lowlevel.HANDLER_RESULT_NOT_YET_HANDLED

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

		return dbus.lowlevel.HANDLER_RESULT_NOT_YET_HANDLED

if __name__ == "__main__":
	dbus.mainloop.glib.DBusGMainLoop (set_as_default = True)
	loop = gobject.MainLoop ()
	bus = IBusServer ()
	print "IBUS_ADDRESS=\"%s\"" % bus.get_address ()
	loop.run ()

