import gobject

class Object (gobject.GObject):
	__gsignals__ = {
		'destroy' : (
			gobject.SIGNAL_RUN_FIRST, 
			gobject.TYPE_NONE,
			())
	}

	def __init__ (self):
		gobject.GObject.__init__ (self)
		self._destroyed = False

	def destroy (self):
		if not self._destroyed:
			self.emit ("destroy")
			self._destroyed = True

gobject.type_register (Object)
