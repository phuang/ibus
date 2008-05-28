import gtk
import gtk.gdk as gdk
import gobject
import pango

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
		self._tooltips = gtk.Tooltips ()
		
		self._orientation = gtk.ORIENTATION_HORIZONTAL
		self._orientation = gtk.ORIENTATION_VERTICAL
		self._show_preedit_string = True
		self._preedit_string = "preedit string"
		self._preedit_attrs = pango.AttrList ()
		self._aux_string = "aux string"
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
		self._preedit_label.set_property ("xalign", 0.0)
		self._preedit_label.set_property ("xpad", 8)
		self._preedit_label.set_no_show_all (True)
		if self._show_preedit_string:
			self._preedit_label.show ()

		# create aux label
		self._aux_label = gtk.Label (self._aux_string)
		self._aux_label.set_attributes (self._aux_attrs)
		self._aux_label.set_property ("xalign", 0.0)
		self._aux_label.set_property ("xpad", 8)
		self._tooltips.set_tip (self._aux_label, "Aux string")

		# create candidates area
		self._candidates_area = gtk.HBox ()
		self._candidates_area.pack_start (gtk.Label ("Candidates Area"))

		# create state label
		self._state_label = gtk.Label ()

		# create buttons
		self._prev_button = gtk.Button ()
		self._prev_button.set_relief (gtk.RELIEF_NONE)
		self._tooltips.set_tip (self._prev_button, "Previous candidate")
		
		self._next_button = gtk.Button ()
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
			self.pack_start (vbox, False, False, 4)
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

			vbox = gtk.VBox ()
			vbox.pack_start (self._preedit_label, False, False, 0)
			vbox.pack_start (self._aux_label, False, False, 0)
			self.pack_start (vbox, False, False, 4)
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

	def set_aux_string (self, text, attrs):
		self._aux_string = text
		self._aux_label.set_text (text)
		if attrs == None:
			attrs = pango.AttrList ()
		self._aux_attrs = attrs
		self._aux_label.set_attributes (attrs)
	
	def set_preedit_string (self, text, attrs):
		self._preedit_string = text
		self._preedit_label.set_text (text)
		if attrs == None:
			attrs = pango.AttrList ()
		self._preedit_attrs = attrs
		self._preedit_label.set_attributes (attrs)

	def set_lookup_table (self, lookup_table):
		self._lookup_table = lookup_table
		candidates = self._lookup_table.get_canidates_in_current_page ()

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

