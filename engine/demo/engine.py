#!/usr/bin/env python
import gobject
import gtk
from ibus import keysyms
from ibus import interface

class Engine (interface.IEngine):
	def __init__ (self, dbusconn, object_path):
		interface.IEngine.__init__ (self, dbusconn, object_path)
		self._dbusconn = dbusconn

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

	def FocusOut (self):
		print "FocusOut"

	def Reset (self):
		print "Reset"

	def SetEnable (self, enable):
		self._enable = enable

	def Destroy (self):
		print "Destroy"

	def SetCursorLocation (self, x, y, w, h):
		pass

class DemoEngine (Engine):
	pass

