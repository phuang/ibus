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


def launch_engine ():
	dbus.mainloop.glib.DBusGMainLoop (set_as_default=True)
	IMApp ().run ()

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

	launch_engine ()

if __name__ == "__main__":
	main ()
