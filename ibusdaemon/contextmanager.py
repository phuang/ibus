import ibus
from inputcontext import InputContext

class ContextManager (ibus.Object):
	def __init__ (self):
		self._contexts = {}

	def create_input_context (self, name, ibusconn):
		context = InputContext (name, ibusconn)
		self._contexts[context.get_id ()] = context
		context.connect ("destroy", self._context_destroy_cb)
		return context

	def release_input_context (self, ic, ibusconn):
		del self._contexts[ic]

	def lookup_context (self, ic, ibusconn):
		return self._contexts[ic]

	def _context_destroy_cb (self, context):
		del self._clients[context.get_id ()]

