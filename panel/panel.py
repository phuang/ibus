import gtk
import ibus
from ibus import interface

class DefaultPanel:
	def __init__ (self):
		pass

	def _create_ui (self):
		pass

	def destroy (self):
		gtk.main_quit ()

class PanelProxy (interface.IPanel):
	def __init__ (self, dbusconn, object_path):
		interface.IPanel.__init__ (self, dbusconn, object_path)
		self._dbusconn = dbusconn
		self._panel = DefaultPanel ()

	def Destroy (self):
		self._panel.destroy ()

