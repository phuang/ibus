import gtk
import gtk.gdk as gdk
import gobject

class Handle (gtk.EventBox):
	def __init__ (self):
		gtk.EventBox.__init__ (self)
		self.set_visible_window (False)
		self.set_size_request (10, -1)
		self.set_events (
			gdk.EXPOSURE_MASK | \
			gdk.BUTTON_PRESS_MASK | \
			gdk.BUTTON_RELEASE_MASK | \
			gdk.BUTTON1_MOTION_MASK)

	def do_button_press_event (self, event):
		if event.button == 1:
			self._press_pos = event.x_root, event.y_root
			self.window.set_cursor (gdk.Cursor (gdk.FLEUR))
			return True	
		return gtk.EventBox.do_button_press_event (self, event)

	def do_button_release_event (self, event):
		if event.button == 1:
			del self._press_pos
			self.window.set_cursor (gdk.Cursor (gdk.LEFT_PTR))
			return True

		return gtk.EventBox.do_button_release_event (self, event)
		
	def do_motion_notify_event (self, event):
		toplevel = self.get_toplevel ()
		x, y = toplevel.get_position ()
		x  = int (x + event.x_root - self._press_pos[0])
		y  = int (y + event.y_root - self._press_pos[1])
		toplevel.move (x, y)
		self._press_pos = event.x_root, event.y_root

	def do_expose_event (self, event):
		self.style.paint_handle (
					self.window,
					gtk.STATE_NORMAL,
					gtk.SHADOW_OUT,
					event.area,
					self,
					"",
					self.allocation.x, self.allocation.y, 
					10, self.allocation.height,
					gtk.ORIENTATION_VERTICAL)
		return True

gobject.type_register (Handle, "IBusHandle")


