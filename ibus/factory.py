# vim:set et sts=4 sw=4:
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

__all__ = (
        "EngineFactoryBase",
    )

import ibus
from ibus import interface

class EngineFactoryBase(ibus.Object):
    def __init__(self, info, engine_class, engine_path, _ibus, object_path):
        super(EngineFactoryBase, self).__init__()
        self.__proxy = EngineFactoryProxy (self, _ibus.get_conn(), object_path)
        self.__info = info
        self.__ibus = _ibus
        self.__engine_class = engine_class
        self.__engine_path = engine_path
        self.__engine_id = 1
        self.__object_path = object_path

    def get_info(self):
        return self.__info

    def initialize(self):
        pass

    def uninitialize(self):
        pass

    def register(self):
        self.__ibus.register_factories([self.__object_path])

    def create_engine(self):
        engine = self.__engine_class(self.__ibus, self.__engine_path + str(self.__engine_id))
        self.__engine_id += 1
        return engine.get_dbus_object()

    def do_destroy(self):
        self.__proxy = None
        self.__ibus = None
        self.__info = None
        self.__engine_class = None
        self.__engine_path = None
        super(EngineBase,self).do_destroy()


class EngineFactoryProxy(interface.IEngineFactory):
    def __init__(self, factory, conn, object_path):
        super(EngineFactoryProxy, self).__init__(conn, object_path)
        self.__factory = factory

    def GetInfo(self):
        return self.__factory.get_info()

    def Initialize(self):
        return self.__factory.initialize()

    def Uninitialize(self):
        return self.__factory.uninitialize()

    def CreateEngine(self):
        return self.__factory.create_engine()

    def Destroy(self):
        self.__factory.destroy()
        self.__factory = None
        self.remove_from_connection ()

