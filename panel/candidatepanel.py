# -*- coding: utf-8 -*-
import gtk
import gtk.gdk as gdk
import gobject
import pango
import ibus
from ibus.gtk import PangoAttrList

class HSeparator (gtk.HBox):
	def __init__ (self):
		gtk.HBox.__init__ (self)
		self.pack_start (gtk.HSeparator (), True, True, 4)

class VSeparator (gtk.VBox):
	def __init__ (self):
		gtk.VBox.__init__ (self)
		self.pack_start (gtk.VSeparator (), True, True, 4)

class CandidateArea (gtk.HBox):
	def __init__ (self, orientation):
		gtk.HBox.__init__ (self)
		self._orientation = orientation
		self._labels = []
		self._create_ui ()

	def _create_ui (self):
		if self._orientation == gtk.ORIENTATION_VERTICAL:
			self._vbox1 = gtk.VBox ()
			self._vbox2 = gtk.VBox ()
			self.pack_start (self._vbox1, False, False, 4)
			self.pack_start (VSeparator(), False, False, 0)
			self.pack_start (self._vbox2, True, True, 4)

		for i in xrange (1, 10):
			label1 = gtk.Label ("%d." % i)
			label1.set_alignment (0.0, 0.5)
			label1.set_no_show_all (True)

			label2 = gtk.Label ()
			label2.set_alignment (0.0, 0.5)
			label2.set_no_show_all (True)

			if self._orientation == gtk.ORIENTATION_VERTICAL:
				label1.set_property ("xpad", 8)
				label2.set_property ("xpad", 8)
				self._vbox1.pack_start (label1, False, False, 2)
				self._vbox2.pack_start (label2, False, False, 2)
			else:
				hbox = gtk.HBox ()
				hbox.pack_start (label1, False, False, 1)
				hbox.pack_start (label2, False, False, 1)
				self.pack_start (hbox, False, False, 4)

			self._labels.append ((label1, label2))

		self._labels[0][0].show ()
		self._labels[0][1].show ()

	def set_candidates (self, candidates, focus_candidate = 0):
		assert len (candidates) <= len (self._labels)
		i = 0
		for text, attrs in candidates:
			self._labels[i][1].set_text (text)
			self._labels[i][1].set_attributes (attrs)
			self._labels[i][0].show ()
			self._labels[i][1].show ()
			if i == focus_candidate:
				self._labels[i][0].set_state (gtk.STATE_SELECTED)
				self._labels[i][1].set_state (gtk.STATE_SELECTED)
			else:
				self._labels[i][0].set_state (gtk.STATE_NORMAL)
				self._labels[i][1].set_state (gtk.STATE_NORMAL)

			i += 1

		for label1, label2 in self._labels[max (1, len(candidates)):]:
			label1.hide ()
			label2.hide ()

		if len (candidates) == 0:
			self._labels[0][0].set_text ("")
			self._labels[0][1].set_text ("")
		else:
			self._labels[0][0].set_text ("1.")

class CandidatePanel (gtk.VBox):
	__gproperties__ = {
		'orientation' : (gtk.Orientation,		# type
		'orientation of candidates',			# nick name
		'the orientation of candidates list',	# description
		0,
		gobject.PARAM_READWRITE)				# flags
		}

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
		gtk.VBox.__init__ (self)
		self._tooltips = gtk.Tooltips ()

		self._toplevel = gtk.Window (gtk.WINDOW_POPUP)
		self._toplevel.add (self)
		self._toplevel.add_events (
			gdk.BUTTON_PRESS_MASK | \
			gdk.BUTTON_RELEASE_MASK | \
			gdk.BUTTON1_MOTION_MASK)
		self._begin_move = False
		self._toplevel.connect ("button-press-event", self._button_press_event_cb)
		self._toplevel.connect ("button-release-event", self._button_release_event_cb)
		self._toplevel.connect ("motion-notify-event", self._motion_notify_event_cb)

		self._orientation = gtk.ORIENTATION_HORIZONTAL
		self._orientation = gtk.ORIENTATION_VERTICAL
		self._show_preedit_string = False
		self._show_aux_string = False
		self._show_lookup_table = False
		self._preedit_string = ""
		self._preedit_attrs = pango.AttrList ()
		self._aux_string = ""
		self._aux_attrs = pango.AttrList ()
		self._lookup_table = None

		self._recreate_ui ()

	def _recreate_ui (self):
		for w in self:
			self.remove (w)
			w.destroy ()
		# create preedit label
		self._preedit_label = gtk.Label (self._preedit_string)
		self._preedit_label.set_attributes (self._preedit_attrs)
		self._preedit_label.set_alignment (0.0, 0.5)
		self._preedit_label.set_padding (8, 0)
		self._preedit_label.set_no_show_all (True)
		if self._show_preedit_string:
			self._preedit_label.show ()

		# create aux label
		self._aux_label = gtk.Label (self._aux_string)
		self._aux_label.set_attributes (self._aux_attrs)
		self._aux_label.set_alignment (0.0, 0.5)
		self._aux_label.set_padding (8, 0)
		self._tooltips.set_tip (self._aux_label, "Aux string")
		self._aux_label.set_no_show_all (True)
		if self._show_aux_string:
			self._aux_label.show ()

		# create candidates area
		self._candidate_area = CandidateArea (self._orientation)
		self._candidate_area.set_no_show_all (True)
		self.update_lookup_table (self._lookup_table, self._show_lookup_table)

		# create state label
		self._state_label = gtk.Label ()
		self._state_label.set_size_request (20, -1)

		# create buttons
		self._prev_button = gtk.Button ()
		self._prev_button.connect ("clicked", lambda x: self.emit ("cursor-up"))
		self._prev_button.set_relief (gtk.RELIEF_NONE)
		self._tooltips.set_tip (self._prev_button, "Previous candidate")

		self._next_button = gtk.Button ()
		self._next_button.connect ("clicked", lambda x: self.emit ("cursor-down"))
		self._next_button.set_relief (gtk.RELIEF_NONE)
		self._tooltips.set_tip (self._next_button, "Next candidate")

		self._pack_all_widgets ()

	def _pack_all_widgets (self):
		if self._orientation == gtk.ORIENTATION_VERTICAL:
			# package all widgets in vertical mode
			image = gtk.Image ()
			image.set_from_stock (gtk.STOCK_GO_UP, gtk.ICON_SIZE_MENU)
			self._prev_button.set_image (image)

			image = gtk.Image ()
			image.set_from_stock (gtk.STOCK_GO_DOWN, gtk.ICON_SIZE_MENU)
			self._next_button.set_image (image)
			vbox = gtk.VBox ()
			vbox.pack_start (self._preedit_label, False, False, 0)
			vbox.pack_start (self._aux_label, False, False, 0)
			self.pack_start (vbox, False, False, 5)
			self.pack_start (HSeparator (), False, False)
			self.pack_start (self._candidate_area, False, False, 2)
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

			vbox = gtk.VBox ()
			vbox.pack_start (self._preedit_label, False, False, 0)
			vbox.pack_start (self._aux_label, False, False, 0)
			self.pack_start (vbox, False, False, 5)
			self.pack_start (HSeparator (), False, False)
			hbox= gtk.HBox ()
			hbox.pack_start (self._candidate_area, True, True, 2)
			hbox.pack_start (VSeparator (), False, False)
			hbox.pack_start (self._prev_button, False, False, 2)
			hbox.pack_start (self._next_button, False, False, 2)
			self.pack_start (hbox, False, False)

		# self.hide_all ()
		# self.show_all ()

	def show_preedit_string (self):
		self._show_preedit_string = True
		self._preedit_label.show ()
		self._check_show_states ()

	def hide_preedit_string (self):
		self._show_preedit_string = False
		self._preedit_label.hide ()
		self._check_show_states ()

	def update_preedit (self, text, attrs, cursor_pos, show):
		attrs = PangoAttrList (attrs, text)
		if show:
			self.show_preedit_string ()
		else:
			self.hide_preedit_string ()
		self._preedit_string = text
		self._preedit_label.set_text (text)
		if attrs == None:
			attrs = pango.AttrList ()
		self._preedit_attrs = attrs
		self._preedit_label.set_attributes (attrs)

	def show_aux_string (self):
		self._show_aux_string = True
		self._aux_label.show ()
		self._check_show_states ()

	def hide_aux_string (self):
		self._show_aux_string = False
		self._aux_label.hide ()
		self._check_show_states ()

	def update_aux_string (self, text, attrs, show):
		attrs = PangoAttrList (attrs, text)

		if show:
			self.show_aux_string ()
		else:
			self.hide_aux_string ()

		self._aux_string = text
		self._aux_label.set_text (text)
		if attrs == None:
			attrs = pango.AttrList ()
		self._aux_attrs = attrs
		self._aux_label.set_attributes (attrs)

	def show_lookup_table (self):
		self._show_lookup_table = True
		self._candidate_area.set_no_show_all (False)
		self._candidate_area.show_all ()
		self._check_show_states ()

	def hide_lookup_table (self):
		self._show_lookup_table = False
		self._candidate_area.hide_all ()
		self._candidate_area.set_no_show_all (True)
		self._check_show_states ()

	def update_lookup_table (self, lookup_table, show):
		if lookup_table == None:
			lookup_table = ibus.LookupTable ()

		if show:
			self.show_lookup_table ()
		else:
			self.hide_lookup_table ()

		self._lookup_table = lookup_table
		candidates = self._lookup_table.get_canidates_in_current_page ()
		candidates = map (lambda x: (x[0], PangoAttrList (x[1], x[0])), candidates)
		self._candidate_area.set_candidates (candidates, self._lookup_table.get_cursor_pos_in_current_page ())

	def _check_show_states (self):
		if self._show_preedit_string or \
			self._show_aux_string or \
			self._show_lookup_table:
			self.show_all ()
			self.emit ("show")
		else:
			self.hide_all ()
			self.emit ("hide")

	def reset (self):
		self.hide ()
		self.hide_preedit_string ()
		self.hide_aux_string ()
		self.hide_lookup_table ()
		self.update_preedit ("", None, 0, False)
		self.update_aux_string ("", None, False)
		self.update_lookup_table (None, False)

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

	def do_size_request (self, requisition):
		gtk.VBox.do_size_request (self, requisition)
		self._toplevel.resize (1, 1)

	def _button_press_event_cb (self, widget, event):
		if event.button == 1:
			self._begin_move = True
			self._press_pos = event.x_root, event.y_root
			self._toplevel.window.set_cursor (gdk.Cursor (gdk.FLEUR))
			return True

		if event.button == 3:
			if self.get_orientation () == gtk.ORIENTATION_HORIZONTAL:
				self.set_orientation (gtk.ORIENTATION_VERTICAL)
			else:
				
				self.set_orientation (gtk.ORIENTATION_HORIZONTAL)
			return True
		return False

	def _button_release_event_cb (self, widget, event):
		if event.button == 1:
			del self._press_pos
			self._begin_move = False
			self._toplevel.window.set_cursor (gdk.Cursor (gdk.LEFT_PTR))
			return True
		return False

	def _motion_notify_event_cb (self, widget, event):
		if self._begin_move != True:
			return False
		x, y = self._toplevel.get_position ()
		x  = int (x + event.x_root - self._press_pos[0])
		y  = int (y + event.y_root - self._press_pos[1])
		self._toplevel.move (x, y)
		self._press_pos = event.x_root, event.y_root
		return True

	def show_all (self):
		gtk.VBox.show_all (self)
		self._toplevel.show_all ()
	
	def hide_all (self):
		gtk.VBox.hide_all (self)
		self._toplevel.hide_all ()

	def move (self, x, y):
		self._toplevel.move (x, y)

gobject.type_register (CandidatePanel, "IBusCandidate")

