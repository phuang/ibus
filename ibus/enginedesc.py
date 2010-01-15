# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2007-2010 Red Hat, Inc.
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
        "EngineDesc",
    )

import dbus
from exception import IBusException
from serializable import *

class EngineDesc(Serializable):
    __gtype_name__ = "PYIBusEngineDesc"
    __NAME__ = "IBusEngineDesc"
    def __init__(self, name="", longname="", description="", language="", license="", author="", icon="", layout="", rank=0):
        super(EngineDesc, self).__init__()
        self.__name = name
        self.__longname = longname
        self.__description = description
        self.__language = language
        self.__license = license
        self.__author = author
        self.__icon = icon
        self.__layout = layout
        self.__rank = rank;

    def get_name(self):
        return self.__name

    def get_longname(self):
        return self.__longname

    def get_description(self):
        return self.__description

    def get_language(self):
        return self.__language

    def get_license(self):
        return self.__license

    def get_author(self):
        return self.__author

    def get_icon(self):
        return self.__icon

    def get_layout(self):
        return self.__layout

    def get_rank(self):
        return self.__rank

    name        = property(get_name)
    longname    = property(get_longname)
    description = property(get_description)
    language    = property(get_language)
    license     = property(get_license)
    author      = property(get_author)
    icon        = property(get_icon)
    layout      = property(get_layout)
    rank        = property(get_rank)

    def serialize(self, struct):
        super(EngineDesc, self).serialize(struct)
        struct.append(dbus.String(self.__name))
        struct.append(dbus.String(self.__longname))
        struct.append(dbus.String(self.__description))
        struct.append(dbus.String(self.__language))
        struct.append(dbus.String(self.__license))
        struct.append(dbus.String(self.__author))
        struct.append(dbus.String(self.__icon))
        struct.append(dbus.String(self.__layout))
        struct.append(dbus.UInt32(self.__rank))

    def deserialize(self, struct):
        super(EngineDesc, self).deserialize(struct)
        self.__name = struct.pop(0)
        self.__longname = struct.pop(0)
        self.__description = struct.pop(0)
        self.__language = struct.pop(0)
        self.__license = struct.pop(0)
        self.__author = struct.pop(0)
        self.__icon = struct.pop(0)
        self.__layout = struct.pop(0)
        self.__rank = struct.pop(0)

def test():
    engine = EngineDesc("Hello", "", "", "", "", "", "", "")
    value = serialize_object(engine)
    engine = deserialize_object(value)

if __name__ == "__main__":
    test()
