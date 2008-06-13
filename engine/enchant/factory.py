from ibus import interface
import engine

FACTORY_PATH = "/com/redhat/IBus/engines/Demo/Factory"
ENGINE_PATH = "/com/redhat/IBus/engines/Demo/Engine/%d"

class DemoEngineFactory (interface.IEngineFactory):
	NAME = "DemoEngine"
	LANG = "en"
	ICON = ""
	AUTHORS = "Huang Peng <shawn.p.huang@gmail.com>"
	CREDITS = "GPLv2"

	def __init__ (self, dbusconn):
		interface.IEngineFactory.__init__ (self, dbusconn, object_path = FACTORY_PATH)
		self._dbusconn = dbusconn
		self._max_engine_id = 1
	
	def GetInfo (self):
		return [
			self.NAME,
			self.LANG,
			self.ICON,
			self.AUTHORS,
			self.CREDITS
			]

	def CreateEngine (self):
		engine_path = ENGINE_PATH % self._max_engine_id
		self._max_engine_id += 1
		return engine.DemoEngine (self._dbusconn, engine_path)


