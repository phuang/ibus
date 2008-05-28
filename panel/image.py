import gtk
import gtk.gdk as gdk

class Image (gtk.Image):
	def __init__ (self, filename, width, height):
		gtk.Image.__init__ (self)
		icon = gtk.IconSource
		pixbuf = gdk.pixbuf_new_from_file_at_scale (filename, width, height, True)
		self.set_from_pixbuf (pixbuf)

