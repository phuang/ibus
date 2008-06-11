import gtk
import gtk.gdk as gdk
import gobject
import ibus
from image import Image
from handle import Handle

ICON_SIZE = gtk.ICON_SIZE_MENU

class LanguageBar (gtk.Toolbar):
	__gsignals__ = {
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, )),
		}
	def __init__ (self):
		gtk.Toolbar.__init__ (self)
		self.set_property ("icon-size", ICON_SIZE)
		icon_theme = gtk.icon_theme_get_default ()
		icon_theme.prepend_search_path ("/home/phuang/sources/ibus/icons")
		# self.set_orientation (gtk.ORIENTATION_VERTICAL)
		self._create_ui ()

		self._properties = {}

	def _add_items (self):
		img = gtk.image_new_from_icon_name ("engine-default", ICON_SIZE)
		btn = gtk.ToolButton (img, "engine")
		btn.connect ("clicked", lambda x: self._add_items ())
		self.insert (btn, -1)

		img = gtk.image_new_from_icon_name ("ibus-keyboard", ICON_SIZE)
		btn = gtk.ToolButton (img, "keyboard")
		self.insert (btn, -1)

		img = gtk.image_new_from_icon_name ("ibus-zh", ICON_SIZE)
		btn = gtk.ToolButton (img, "keyboard")
		self.insert (btn, -1)

		self.insert (gtk.SeparatorToolItem (), -1)
		self.show_all ()
		self.check_resize ()

	def _create_ui (self):
		# create move handle
		self._handle = gtk.ToolItem ()
		self._handle.add (Handle ())
		self.insert (self._handle, -1)

		# create input methods menu
		self._im_menu = gtk.ToolButton (icon_widget = gtk.image_new_from_icon_name ("ibus-keyboard", gtk.ICON_SIZE_MENU))
		self.insert (self._im_menu, -1)

	def do_show (self):
		gtk.Toolbar.do_show (self)
		self.check_resize ()

	def do_check_resize (self):
		width = 0
		for item in self:
			w, h = item.size_request ()
			width += w
		self.set_size_request (width + 4, -1)

	def reset (self):
		self._remove_properties ()

	def register_properties (self, props):
		self._remove_properties ()
		# create new properties
		for prop in props:
			if prop._type == ibus.PROP_TYPE_NORMAL:
				widget = gtk.ToolButton ()
				widget.set_icon_name (prop._icon)
				widget.set_label (prop._label)
				widget.connect ("clicked",
						lambda widget, prop: self.emit ("property-activate", prop._name),
						prop)
			else:
				widget = gtk.ToolItem ()

			widget.set_sensitive (prop._sensitive)
			if prop._visible:
				widget.set_no_show_all (False)
				widget.show ()
			else:
				widget.set_no_show_all (True)
				widget.hide ()
			if not self._properties.has_key (prop._name):
				self._properties [prop._name] = []
			self._properties [prop._name].append ((prop, widget))
			self.insert (widget, -1)
		self.check_resize ()

	def update_properties (self, props):
		pass

	def _remove_properties (self):
		# reset all properties
		for name, props in self._properties.items ():
			for prop, widget in props:
				widget.hide ()
				widget.destroy ()
		self._properties = {}
		self.check_resize ()

gobject.type_register (LanguageBar, "IBusLanguageBar")

class LanguageBarWindow (gtk.Window):
	__gsignals__ = {
		"property-activate" : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_STRING, )),
		}
	def __init__ (self):
		gtk.Window.__init__ (self, gtk.WINDOW_POPUP)
		self._language_bar = LanguageBar ()
		self._language_bar.connect ("size-request", self._size_request_cb)
		self._language_bar.connect ("property-activate",
								lambda widget, prop_name: self.emit ("property-activate", prop_name))
		self.add (self._language_bar)
		self.show_all ()

	def reset (self):
		self._language_bar.reset ()

	# def do_size_allocate (self, allocation):
	#		gtk.Window.do_size_allocate (self, allocation)
	#		root = gdk.get_default_root_window ()
	#		workarea = root.property_get ("_NET_WORKAREA")[2]
	#		x, y = workarea[2] - allocation.width - 40, workarea[1] + workarea[3] - allocation.height
	#		self.move (x, y)

	def do_destroy (self):
		gtk.main_quit ()
		gtk.Window.do_destroy (self)

	def register_properties (self, props):
		self._language_bar.register_properties (props)

	def update_property (self, prop):
		self._labguage_bar.update_property (prop)

	def _size_request_cb (self, widget, requisition):
		self.resize (1, 1)

gobject.type_register (LanguageBarWindow, "IBusLanguageBarWindow")

