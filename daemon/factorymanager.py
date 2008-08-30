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

import weakref
import gobject
import ibus
from enginefactory import EngineFactory

class FactoryManager(ibus.Object):
    __gsignals__ = {
        'new-factories-added' : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_PYOBJECT, )
        )
    }

    def __init__(self):
        super(FactoryManager, self).__init__()
        self.__factories = {}
        self.__ibusconn_factory_dict = {}
        self.__default_factory = None
        self.__sorted_factories = None

    def register_factories(self, object_paths, ibusconn):
        if ibusconn in self.__factories:
            raise ibus.IBusException("this conn has registered factories!")

        self.__ibusconn_factory_dict[ibusconn] = []

        for object_path in object_paths:
            if object_path in self.__factories:
                raise ibus.IBusException(
                        "Factory [%s] has been registered!" % object_path)

            factory = EngineFactory(ibusconn, object_path)
            self.__factories[object_path] = factory
            self.__ibusconn_factory_dict[ibusconn].append(object_path)
        self.__sorted_factories = None

        ibusconn.connect("destroy", self.__ibusconn_destroy_cb)

        self.emit("new-factories-added",
                    self.__ibusconn_factory_dict[ibusconn][:])

    def get_default_factory(self):
        if self.__default_factory == None:
            factories = self.__get_sorted_factories()
            if factories:
                self.__default_factory = factories[0]

        return self.__default_factory

    def set_default_factory(self, factory):
        if factory in self.__get_sorted_factories():
            self.__default_factory = factory
        else:
            print "unknown factory"

    def get_next_factory(self, factory):
        factories = self.__get_sorted_factories()
        i = factories.index(factory) + 1
        if i >= len(factories):
            i = 0

        return factories[i]

    def get_factories(self):
        return self.__factories.keys()

    def get_factory_info(self, factory_path):
        factory = self.__factories[factory_path]
        return factory.get_info()

    def get_factory(self, factory_path):
        factory = self.__factories[factory_path]
        return factory

    def __get_sorted_factories(self, resort = False):
        if not self.__sorted_factories or resort:
            factories = self.__factories.values()
            factories.sort()
            self.__sorted_factories = factories
        return self.__sorted_factories

    def __ibusconn_destroy_cb(self, ibusconn):
        assert ibusconn in self.__ibusconn_factory_dict

        for object_path in self.__ibusconn_factory_dict[ibusconn]:
            factory = self.__factories[object_path]
            if factory == self.__default_factory:
                self.__default_factory = None
            del self.__factories[object_path]

        del self.__ibusconn_factory_dict[ibusconn]
        self.__sorted_factories = None

gobject.type_register(FactoryManager)

