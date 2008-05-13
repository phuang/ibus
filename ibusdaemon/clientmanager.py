import ibus
from client import Client

class ClientManager (ibus.Object):
	def __init__ (self):
		self._clients = {}

	def register_client (self, name, ibusconn):
		if ibusconn in self._clients:
			raise ibus.IBusException ("client has been registered")
		client = Client (name, ibusconn)
		self._clients[ibusconn] = client
		ibusconn.connect ("destroy", self._ibusconn_destroy_cb)
		return client

	def lookup_client (self, ibusconn):
		return self._clients[ibusconn]

	def _ibusconn_destroy_cb (self, ibusconn):
		del self._clients[ibusconn]

