#!/usr/bin/env python

import dbus
import dbus.server
import dbus.lowlevel
import dbus.service
import dbus.mainloop.glib
import gobject

class DBusObject (dbus.service.Object):
	SUPPORTS_MULTIPLE_CONNECTIONS = True
	def __init__ (self):
		dbus.service.Object.__init__ (self)
		self._max_id = 1

	@dbus.service.method (dbus_interface=dbus.BUS_DAEMON_IFACE, out_signature="s", connection_keyword="connection")
	def Hello (self, connection):
		print "Hello is called"
		name = "ibus.%d" % self._max_id
		self._max_id = self._max_id +1
		connection.set_unique_name (name)
		return name


