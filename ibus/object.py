import gobject

class Object (gobject.GObject):
	__gsignals__ = {
		'destroy' : (
			gobject.SIGNAL_RUN_FIRST, 
			gobject.TYPE_NONE,
			())
	}

	def destroy (self):
		self.emit ("destroy")

gobject.type_register (Object)
