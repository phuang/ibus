import gtk
import gtk.gdk as gdk
import gobject
import ibus
from candidatepanel import CandidatePanel

class CandidateWindow (gtk.Window):
	__gsignals__ = {
		"cursor-up" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
		"cursor-down" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			()),
	}

	def __init__ (self):
		gtk.Window.__init__ (self, gtk.WINDOW_POPUP)
		self.add_events (
			gdk.BUTTON_PRESS_MASK | \
			gdk.BUTTON_RELEASE_MASK | \
			gdk.BUTTON1_MOTION_MASK)

		self._candidate_panel = CandidatePanel ()
		self._begin_move = False
		self._candidate_panel.connect ("size-request", self._size_request_cb)
		self._candidate_panel.connect ("cursor-up", lambda x: self.emit ("cursor-up"))
		self._candidate_panel.connect ("cursor-down", lambda x: self.emit ("cursor-down"))
		self._candidate_panel.connect ("show", lambda x: self.show ())
		self._candidate_panel.connect ("hide", lambda x: self.hide ())
		self.add (self._candidate_panel)
		self.move (100, 100)

	def update_preedit (self, text, attrs, cursor_pos, show):
		self._candidate_panel.update_preedit (text, attrs, cursor_pos, show)

	def update_aux_string (self, text, attrs, show):
		self._candidate_panel.update_aux_string (text, attrs, show)

	def update_lookup_table (self, lookup_table, show):
		self._candidate_panel.update_lookup_table (lookup_table, show)

	def reset (self):
		self.hide ()
		self._candidate_panel.reset ()

	def _size_request_cb (self, widget, size):
		self.resize (1, 1)

	def _change_orientation (self):
		if self._candidate_panel.get_orientation () == gtk.ORIENTATION_HORIZONTAL:
			self._candidate_panel.set_orientation (gtk.ORIENTATION_VERTICAL)
		else:
			self._candidate_panel.set_orientation (gtk.ORIENTATION_HORIZONTAL)

	def do_button_press_event (self, event):
		if event.button == 1:
			self._begin_move = True
			self._press_pos = event.x_root, event.y_root
			self.window.set_cursor (gdk.Cursor (gdk.FLEUR))
			return True

		if event.button == 3:
			self._change_orientation ()
			return True
		return False

	def do_button_release_event (self, event):
		if event.button == 1:
			del self._press_pos
			self._begin_move = False
			self.window.set_cursor (gdk.Cursor (gdk.LEFT_PTR))
			return True
		return False

	def do_motion_notify_event (self, event):
		if self._begin_move != True:
			return False
		x, y = self.get_position ()
		x  = int (x + event.x_root - self._press_pos[0])
		y  = int (y + event.y_root - self._press_pos[1])
		self.move (x, y)
		self._press_pos = event.x_root, event.y_root
		return True

gobject.type_register (CandidateWindow, "IBusCandidateWindow")

if __name__ == "__main__":
	# style_string = """"""
	# gtk.rc_parse_string (style_string)
	window = CandidateWindow ()
	window.show_all ()
	gtk.main ()

