# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA

from ibus import interface
import engine

FACTORY_PATH = "/com/redhat/IBus/engines/Demo/Factory"
ENGINE_PATH = "/com/redhat/IBus/engines/Demo/Engine/%d"

class DemoEngineFactory (interface.IEngineFactory):
	NAME = "Enchant"
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


