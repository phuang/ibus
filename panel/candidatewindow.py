import gtk
import gtk.gdk as gdk
import gobject
import ibus
from candidatepanel import CandidatePanel

class CandidateWindow (gtk.Window):
	def __init__ (self):
		gtk.Window.__init__ (self, gtk.WINDOW_POPUP)
		self.add_events (
			gdk.BUTTON_PRESS_MASK | \
			gdk.BUTTON_RELEASE_MASK | \
			gdk.BUTTON1_MOTION_MASK)

		self._candidate_panel = CandidatePanel ()
		self._begin_move = False
		self._candidate_panel.connect ("size-request", self._size_request_cb)
		self.add (self._candidate_panel)
		self.move (100, 100)

	def set_preedit_string (self, text, attrs):
		self._candidate_panel.set_preedit_string (text, attrs)

	def show_preedit_string (self, text, attrs):
		self._candidate_panel.show_preedit_string ()

	def hide_preedit_string (self, text, attrs):
		self._candidate_panel.hide_preedit_string ()

	def set_aux_string (self, text, attrs):
		self._candidate_panel.set_aux_string (text, attrs)

	def set_lookup_table (self, lookup_table):
		self._candidate_pabel.set_lookup_table (lookup_table)

	def _size_request_cb (self, widget, size):
		self.resize (max (size.width, 200), 1)

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
	# gtk.rc_parse ("./themes/default/gtkrc")
	window = CandidateWindow ()
	window.show_all ()
	gtk.main ()

