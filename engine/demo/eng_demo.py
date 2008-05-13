#!/usr/bin/env python
import gobject
import gtk
import dbus.connection
import dbus.mainloop.glib
from ibus import keysyms
from ibus import interface
from ibus.common import \
	IBUS_NAME, \
	IBUS_IFACE, \
	IBUS_PATH, \
	IBUS_ADDR

IBUS_DEMO_ENGINE_FACTORY_PATH = "/com/redhat/IBus/engines/Demo/Factory"
IBUS_DEMO_ENGINE_PATH = "/com/redhat/IBus/engines/Demo/Engine/%d"

class LookupTable (gtk.Window):
	def __init__ (self):
		gtk.Window.__init__ (self, gtk.WINDOW_POPUP)
		self._enable = False
		# create ui
		vbox = gtk.VBox ()
		self._preedit_label = gtk.Label ("preedit string")
		self._candidates_label = gtk.Label ("candidates")
		vbox.pack_start (self._preedit_label)
		vbox.pack_start (self._candidates_label)
		self.add (vbox)

	def focus_in (self):
		if self._enable:
			self.show_all ()

	def focus_out (self):
		if self._enable:
			self.hide_all ()

	def set_enable (self, enable):
		self._enable = enable
		if enable:
			self.show_all ()
		else:
			self.hide_all ()

	def set_cursor_location (self, x, y, w, h):
		self.move (x + w, y + h)

class DemoEngineFactory (interface.IEngineFactory):
	NAME = "DemoEngine"
	LANG = "en"
	ICON = ""
	AUTHORS = "Huang Peng <shawn.p.huang@gmail.com>"
	CREDITS = "GPLv2"

	def __init__ (self, dbusconn):
		interface.IEngineFactory.__init__ (self, dbusconn, object_path = IBUS_DEMO_ENGINE_FACTORY_PATH)
		self._dbusconn = dbusconn
		self._max_engine_id = 1
	
	def GetInfo (self):
		return [
			self.NAME,
			self.LANG,
			self.ICON,
			self.AUTHORS,
			self.CREDITS
			]

	def CreateEngine (self):
		engine_path = IBUS_DEMO_ENGINE_PATH % self._max_engine_id
		self._max_engine_id += 1
		return DemoEngine (self._dbusconn, engine_path)


class DemoEngine (interface.IEngine):

	def __init__ (self, dbusconn, object_path):
		interface.IEngine.__init__ (self, dbusconn, object_path = object_path)
		self._dbusconn = dbusconn
		self._lookup_table = LookupTable ()

	def ProcessKeyEvent (self, keyval, is_press, state):
		if not is_press:
			return False
		if keyval < 128 and (state & keysyms.CONTROL_MASK | keysyms.MOD1_MASK | keysyms.SHIFT_MASK) == 0:
			self.CommitString (chr(keyval))
			print "commit %s" % chr(keyval)
			return True
		return False

	def FocusIn (self):
		print "FocusIn"
		self._lookup_table.focus_in ()

	def FocusOut (self):
		print "FocusOut"
		self._lookup_table.focus_out ()

	def Reset (self):
		print "Reset"

	def SetEnable (self, enable):
		self._enable = enable
		self._lookup_table.set_enable (enable)

	def Destroy (self):
		print "Destroy"
		self._lookup_table.destroy ()

	def SetCursorLocation (self, x, y, w, h):
		self._lookup_table.set_cursor_location (x, y, w, h)
		

class IMApp:
	def __init__ (self):
		self._dbusconn = dbus.connection.Connection (IBUS_ADDR)
		self._dbusconn.add_signal_receiver (self._disconnected_cb, 
							"Disconnected", 
							dbus_interface=dbus.LOCAL_IFACE)
		self._engine = DemoEngineFactory (self._dbusconn)
		self._ibus = self._dbusconn.get_object (IBUS_NAME, IBUS_PATH)
		self._ibus.RegisterFactories ([IBUS_DEMO_ENGINE_FACTORY_PATH])

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
