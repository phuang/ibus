import weakref
import gobject
import ibus

class Panel (ibus.Object):
	def __init__ (self, ibusconn, object_path):
		ibus.Object.__init__ (self)
		self._ibusconn = ibusconn
		self._object_path = object_path
		self._panel = self._ibusconn.get_object (self._object_path)
		self._lookup_table = ibus.LookupTable ()

		self._ibusconn.connect ("destroy", self._ibusconn_destroy_cb)
		self._ibusconn.connect ("dbus-signal", self._dbus_signal_cb)

	def update_lookup_table (self, lookup_table):
		self._lookup_table = lookup_table

	def destroy (self):
		if self._ibusconn != None:
			self._panel.Destroy ()

		self._ibusconn = None
		self._panel = None
		ibus.Object.destroy (self)

	# signal callbacks
	def _ibusconn_destroy_cb (self, ibusconn):
		self._ibusconn = None
		self.destroy ()

	def _dbus_signal_cb (self, ibusconn, message):
		pass

	# methods for cmp
	# def __lt__ (self, other):
	#		x = self.get_info ()
	#		y = other.get_info ()
	#		if x[1] < y[1]: return True
	#		if x[1] == y[1]: return x[0] < y[0]
	#
	#	def __gt__ (self, other):
	#		x = self.get_info ()
	#		y = other.get_info ()
	#		if x[1] > y[1]: return True
	#		if x[1] == y[1]: return x[0] > y[0]

class DummyPanel:
	pass
