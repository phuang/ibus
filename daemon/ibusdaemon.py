# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA

import os
import sys
import getopt
import getpass
import gobject
import dbus
import dbus.server
import dbus.service
import dbus.lowlevel
import dbus.mainloop.glib
import ibus
from bus import IBus, IBusProxy

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
		if name == dbus.BUS_DAEMON_NAME:
			return dbus.BUS_DAEMON_NAME
		elif name == ibus.IBUS_NAME:
			return ibus.IBUS_NAME

		raise dbus.DBusException (
				"org.freedesktop.DBus.Error.NameHasNoOwner: Could not get owner of name '%s': no such name" % name)

	@method (in_signature = "s")
	def AddMatch (self, rule):
		pass

	@signal (signature = "sss")
	def NameOwnerChanged (self, name, old_owner, new_owner):
		pass

class IBusServer(dbus.server.Server):
	def __init__(self, *args, **kargs):
		super(IBusServer, self).__init__ ()

		self._ibus = IBus()

		# self.register_object(self._ibus, ibus.IBUS_PATH)
		#
		# self._dbus = DBus()
		# self.register_object(self._dbus, dbus.BUS_DAEMON_PATH)

	def _on_new_connection (self, dbusconn):
		IBusProxy (self._ibus, dbusconn)

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

def launch_ibus ():
	dbus.mainloop.glib.DBusGMainLoop (set_as_default = True)
	loop = gobject.MainLoop ()
	try:
		os.mkdir ("/tmp/ibus-%s" % getpass.getuser ())
	except:
		pass
	bus = IBusServer (ibus.IBUS_ADDR)
	try:
		loop.run ()
	except KeyboardInterrupt, e:
		print "daemon exits"
		sys.exit ()


def print_help (out, v = 0):
	print >> out, "-h, --help             show this message."
	print >> out, "-d, --daemonize        daemonize ibus"
	sys.exit (v)

def main ():
	daemonize = False
	shortopt = "hd"
	longopt = ["help", "daemonize"]
	try:
		opts, args = getopt.getopt (sys.argv[1:], shortopt, longopt)
	except getopt.GetoptError, err:
		print_help (sys.stderr, 1)

	for o, a in opts:
		if o in ("-h", "--help"):
			print_help (sys.stdout)
		elif o in ("-d", "--daemonize"):
			daemonize = True
		else:
			print >> sys.stderr, "Unknown argument: %s" % o
			print_help (sys.stderr, 1)

	if daemonize:
		if os.fork ():
			sys.exit ()

	launch_ibus ()


if __name__ == "__main__":
	main ()
