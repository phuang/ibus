#!/usr/bin/env python
import os
import sys
import getpass
import gobject
import dbus
import dbus.server
import dbus.service
import dbus.lowlevel
import dbus.mainloop.glib
import ibus
from bus import IBusProxy

class DBus (dbus.service.Object):
	SUPPORTS_MULTIPLE_CONNECTIONS = True

	method = lambda **args: \
		dbus.service.method (dbus_interface = dbus.BUS_DAEMON_IFACE, \
		**args)

	signal = lambda **args: \
		dbus.service.signal (dbus_interface = dbus.BUS_DAEMON_IFACE, \
		**args)

	def __init__ (self):
		dbus.service.Object.__init__ (self)

	@method (in_signature = "s", out_signature = "s")
	def GetNameOwner (self, name):
		print "GetNameOwner (%s)" % name

		if name == dbus.BUS_DAEMON_NAME:
			return dbus.BUS_DAEMON_NAME
		elif name == ibus.IBUS_NAME:
			return ibus.IBUS_NAME

		raise dbus.DBusException (
				"org.freedesktop.DBus.Error.NameHasNoOwner: Could not get owner of name '%s': no such name" % name)

	@method (in_signature = "s")
	def AddMatch (self, rule):
		print "AddMatch"
		pass

	@signal (signature = "sss")
	def NameOwnerChanged (self, name, old_owner, new_owner):
		pass

class IBusServer (dbus.server.Server):
	def __init__ (self):
		dbus.server.Server.__init__ (self, ibus.IBUS_ADDR)

		self._ibus = IBusProxy ()
		self.register_object (self._ibus, ibus.IBUS_PATH)

		self._dbus = DBus ()
		self.register_object (self._dbus, dbus.BUS_DAEMON_PATH)

	def new_connection (self, server, dbusconn):
		dbusconn.add_message_filter (self.message_filter_cb)
		self._ibus.new_connection (dbusconn)

	def remove_connection (self, dbusconn):
		self._ibus.remove_connection (dbusconn)

	def message_filter_cb (self, dbusconn, message):
		# if message.is_method_call (dbus.LOCAL_IFACE, "Disconnected"):
		# 	return dbus.lowlevel.HANDLER_RESULT_NOT_YET_HANDLED

		if message.get_type () == 4: # is signal
			if self._ibus.dispatch_dbus_signal (dbusconn, message):
				return dbus.lowlevel.HANDLER_RESULT_HANDLED

		self._print_message (message)

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
	try:
		os.mkdir ("/tmp/ibus-%s" % getpass.getuser ())
	except:
		pass
	bus = IBusServer ()
	print "IBUS_ADDRESS=\"%s\"" % bus.get_address ()
	try:
		loop.run ()
	except KeyboardInterrupt, e:
		print "daemon exits"
		sys.exit ()

