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

		self._move_begined = False

		root = gdk.get_default_root_window ()
		workarea = root.property_get ("_NET_WORKAREA")[2]

	def do_button_press_event (self, event):
		if event.button == 1:
			root = gdk.get_default_root_window ()
			desktop = root.property_get ("_NET_CURRENT_DESKTOP")[2][0]
			self._workarea = root.property_get ("_NET_WORKAREA")[2][desktop * 4: (desktop + 1) * 4]
			self._move_begined = True
			toplevel = self.get_toplevel ()
			x, y = toplevel.get_position ()
			self._press_pos = event.x_root - x, event.y_root - y
			self.window.set_cursor (gdk.Cursor (gdk.FLEUR))
			return True
		return False

	def do_button_release_event (self, event):
		if event.button == 1:
			self._move_begined = False
			del self._press_pos
			del self._workarea
			self.window.set_cursor (gdk.Cursor (gdk.LEFT_PTR))
			return True

		return False

	def do_motion_notify_event (self, event):
		if not self._move_begined:
			return
		toplevel = self.get_toplevel ()
		x, y = toplevel.get_position ()
		x  = int (event.x_root - self._press_pos[0])
		y  = int (event.y_root - self._press_pos[1])

		if x < self._workarea[0] and x > self._workarea[0] - 16:
			x = self._workarea[0]
		if y < self._workarea[1] and y > self._workarea[1] - 16:
			y = self._workarea[1]

		w, h = toplevel.get_size ()
		if x + w > self._workarea[0] + self._workarea[2] and \
			x + w < self._workarea[0] + self._workarea[2] + 16:
			x = self._workarea[0] + self._workarea[2] - w
		if y + h > self._workarea[1] + self._workarea[3] and \
			y + h < self._workarea[1] + self._workarea[3] + 16:
			y =  self._workarea[1] + self._workarea[3] - h

		toplevel.move (x, y)

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


