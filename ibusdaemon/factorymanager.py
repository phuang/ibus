import weakref
import gobject
import ibus
from enginefactory import EngineFactory

class FactoryManager (ibus.Object):
	__gsignals__ = {
		'new-factories-added' : (
			gobject.SIGNAL_RUN_FIRST,
			gobject.TYPE_NONE,
			(gobject.TYPE_PYOBJECT, )
		)
	}
	def __init__ (self):
		ibus.Object.__init__ (self)
		self._factories = {}
		self._ibusconn_factory_dict = {}
		self._default_factory = None
		self._sorted_factories = None

	def register_factories (self, object_paths, ibusconn):
		if ibusconn in self._factories:
			raise ibus.IBusException ("this conn has registered factories!")

		self._ibusconn_factory_dict[ibusconn] = []

		for object_path in object_paths:
			if object_path in self._factories:
				raise ibus.IBusException (
						"Factory [%s] has been registered!" % object_path)

			factory = EngineFactory (ibusconn, object_path)
			self._factories[object_path] = factory
			self._ibusconn_factory_dict[ibusconn].append (object_path)

		ibusconn.connect ("destroy", self._ibusconn_destroy_cb)

		self.emit ("new-factories-added",
					self._ibusconn_factory_dict[ibusconn][:])

	def get_default_factory (self):
		if self._default_factory == None:
			factories = self._get_sorted_factories ()
			if factories:
				self._default_factory = factories[0]

		return self._default_factory

	def get_next_factory (self, factory):
		factories = self._get_sorted_factories ()
		i = factories.index (factory) + 1
		if i >= len (factories):
			i = 0

		return factories[i]

	def get_factories (self):
		return self._factories.keys ()

	def get_factory_info (self, factory_path):
		factory = self._factories[factory_path]
		return factory.get_info ()

	def get_factory (self, factory_path):
		factory = self._factories[factory_path]
		return factory

	def _get_sorted_factories (self, resort = False):
		if not self._sorted_factories or resort:
			factories = self._factories.values ()
			factories.sort ()
			self._sorted_factories = factories
		return self._sorted_factories

	def _ibusconn_destroy_cb (self, ibusconn):
		assert ibusconn in self._ibusconn_factory_dict

		for object_path in self._ibusconn_factory_dict[ibusconn]:
			factory = self._factories[object_path]
			if factory == self._default_factory:
				self._default_factory = None
			del self._factories[object_path]

		del self._ibusconn_factory_dict[ibusconn]
		self._sorted_factories = None

gobject.type_register (FactoryManager)

