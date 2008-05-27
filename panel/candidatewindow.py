import gtk
import gtk.gdk as gdk
import gobject
import ibus

SPACING = 4

class HSeparator (gtk.HBox):
	def __init__ (self):
		gtk.HBox.__init__ (self)
		self.pack_start (gtk.HSeparator (), True, True, 4)

class VSeparator (gtk.VBox):
	def __init__ (self):
		gtk.VBox.__init__ (self)
		self.pack_start (gtk.VSeparator (), True, True, 4)

class CandidatePanel (gtk.VBox):
	__gproperties__ = {
		'orientation' : (gtk.Orientation,		# type
		'orientation of candidates',			# nick name
		'the orientation of candidates list',	# description
		0,
		gobject.PARAM_READWRITE)				# flags
		}

	def __init__ (self):
		gtk.VBox.__init__ (self)
		
		self._orientation = gtk.ORIENTATION_HORIZONTAL
		self._orientation = gtk.ORIENTATION_VERTICAL
		self._show_preedit_string = False
		self._preedit_string = "preedit string"
		self._aux_string = "aux string"
		self._lookup_table = None
		
		self._recreate_ui ()
		

	def _recreate_ui (self):
		for w in self:
			self.remove (w)
			w.destroy ()
		# create preedit label
		self._preedit_label = gtk.Label (self._preedit_string)
		self._preedit_label.set_property ("xalign", 0.0)
		self._preedit_label.set_property ("xpad", 8)
		self._preedit_label.set_no_show_all (True)

		# create aux label
		self._aux_label = gtk.Label (self._aux_string)
		self._aux_label.set_property ("xalign", 0.0)
		self._aux_label.set_property ("xpad", 8)

		# create candidates area
		self._candidates_area = gtk.HBox ()
		self._candidates_area.pack_start (gtk.Label ("Candidates Area"))

		# create state label
		self._state_label = gtk.Label ()

		# create buttons
		self._prev_button = gtk.Button ()
		self._prev_button.set_relief (gtk.RELIEF_NONE)
		
		self._next_button = gtk.Button ()
		self._next_button.set_relief (gtk.RELIEF_NONE)
		
		self.pack_all_widgets ()

	def pack_all_widgets (self):
		if self._orientation == gtk.ORIENTATION_VERTICAL:		
			# package all widgets in vertical mode
			image = gtk.Image ()
			image.set_from_stock (gtk.STOCK_GO_UP, gtk.ICON_SIZE_MENU)
			self._prev_button.set_image (image)

			image = gtk.Image ()
			image.set_from_stock (gtk.STOCK_GO_DOWN, gtk.ICON_SIZE_MENU)
			self._next_button.set_image (image)
			self.pack_start (self._preedit_label, False, False, 4)
			self.pack_start (self._aux_label, False, False, 4)
			self.pack_start (HSeparator (), False, False)
			self.pack_start (self._candidates_area, False, False, 4)
			self.pack_start (HSeparator (), False, False)
			hbox= gtk.HBox ()
			hbox.pack_start (self._state_label, True, True)
			hbox.pack_start (VSeparator (), False, False)
			hbox.pack_start (self._prev_button, False, False, 2)
			hbox.pack_start (self._next_button, False, False, 2)
			self.pack_start (hbox, False, False)
		else:
			# package all widgets in HORIZONTAL mode
			image = gtk.Image ()
			image.set_from_stock (gtk.STOCK_GO_BACK, gtk.ICON_SIZE_MENU)
			self._prev_button.set_image (image)

			image = gtk.Image ()
			image.set_from_stock (gtk.STOCK_GO_FORWARD, gtk.ICON_SIZE_MENU)
			self._next_button.set_image (image)

			self.pack_start (self._preedit_label, False, False, 4)
			self.pack_start (self._aux_label, False, False, 4)
			self.pack_start (HSeparator (), False, False)
			hbox= gtk.HBox ()
			hbox.pack_start (self._candidates_area, True, True, 4)
			hbox.pack_start (VSeparator (), False, False)
			hbox.pack_start (self._prev_button, False, False, 2)
			hbox.pack_start (self._next_button, False, False, 2)
			self.pack_start (hbox, False, False)
			
		self.hide_all ()
		self.show_all ()


	def show_preedit_string (self):
		self._show_preedit_string = True
		self._preedit_label.show ()

	def hide_preedit_string (self):
		self._hide_preedit_string = False
		self._preedit_label.hide ()

	def set_aux_string (self, text):
		self._aux_string = text
		self._aux_label.set_text (text)
	
	def set_preedit_string (self, text):
		self._preedit_string = text
		self._preedit_label.set_text (text)

	def set_lookup_table (self, lookup_table):
		self._lookup_table = lookup_table
		candidates = self._lookup_table.get_canidates_in_current_page ()
		candidate_layouts = []
		for text, attrs in candidates:
			candidate_layouts.append (self.create_pango_layout (text))

		self._candidate_layouts = candidate_layouts

	def set_orientation (self, orientation):
		if self._orientation == orientation:
			return
		self._orientation = orientation
		self._recreate_ui ()
	
	def get_orientation (self):
		return self._orientation

	def do_set_property (self, property, value):
		if property == 'orientation':
			self.set_orientation (value)
		else:
			return gtk.DrawingArea.do_set_property (property, value)
	
	def do_get_property (self, property):
		if property == 'orientation':
			return self._orientation
		else:
			return gtk.DrawingArea.do_get_property (property)

	def do_expose_event (self, event):
		self.style.paint_box (self.window,
					gtk.STATE_NORMAL,
					gtk.SHADOW_IN,
					event.area,
					self,
					"menu",
					self.allocation.x, self.allocation.y, 
					self.allocation.width, self.allocation.height) 

		gtk.VBox.do_expose_event (self, event)

gobject.type_register (CandidatePanel, "IBusCandidate")

class CandidateWindow (gtk.Window):
	def __init__ (self):
		gtk.Window.__init__ (self, gtk.WINDOW_POPUP)
		self.add_events (
			gdk.BUTTON_PRESS_MASK | \
			gdk.BUTTON_RELEASE_MASK | \
			gdk.BUTTON1_MOTION_MASK)

		self.set_property ("border-width", 2)
		self._candidate_panel = CandidatePanel ()
		self._candidate_panel.connect ("size-request", self._size_request_cb)
		self.add (self._candidate_panel)
		self.move (100, 100)

	def _size_request_cb (self, widget, size):
		self.resize (max (size.width, 200), 1)

	def do_button_press_event (self, event):
		if event.button == 1:
			self._press_pos = event.x_root, event.y_root
			self.window.set_cursor (gdk.Cursor (gdk.FLEUR))
			return True
		if event.button == 3:
			if self._candidate_panel.get_orientation () == gtk.ORIENTATION_HORIZONTAL:
				self._candidate_panel.set_orientation (gtk.ORIENTATION_VERTICAL)
			else:
				self._candidate_panel.set_orientation (gtk.ORIENTATION_HORIZONTAL)

			return True
		return False
	
	def do_button_release_event (self, event):
		if event.button == 1:
			del self._press_pos
			self.window.set_cursor (gdk.Cursor (gdk.LEFT_PTR))
			return True
		return False
	
	def do_motion_notify_event (self, event):
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

