#!/usr/bin/env python
import gobject
import gtk
from ibus import keysyms
from ibus import interface
import panel


class Engine (interface.IEngine):
	def __init__ (self, dbusconn, object_path):
		interface.IEngine.__init__ (self, dbusconn, object_path)
		self._dbusconn = dbusconn
		self._panel = panel.Panel ()
	
	# methods for dbus rpc
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
		self._panel.focus_in ()

	def FocusOut (self):
		print "FocusOut"
		self._panel.focus_out ()

	def Reset (self):
		print "Reset"

	def SetEnable (self, enable):
		self._enable = enable
		self._panel.set_enable (enable)

	def Destroy (self):
		print "Destroy"
		self._panel.destroy ()

	def SetCursorLocation (self, x, y, w, h):
		self._panel.set_cursor_location (x, y, w, h)

class DemoEngine (Engine):
	pass


		


