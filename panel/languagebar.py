import gtk
import gtk.gdk as gdk
import gobject
from image import Image
from handle import Handle

ICON_SIZE = gtk.ICON_SIZE_MENU

class LanguageBar (gtk.Toolbar):
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
		pass

	def register_properties (self, props):
		pass

	def update_properties (self, props):
		pass

gobject.type_register (LanguageBar, "IBusLanguageBar")

class LanguageBarWindow (gtk.Window):
	def __init__ (self):
		gtk.Window.__init__ (self, gtk.WINDOW_POPUP)
		self._language_bar = LanguageBar ()
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

gobject.type_register (LanguageBarWindow, "IBusLanguageBarWindow")

