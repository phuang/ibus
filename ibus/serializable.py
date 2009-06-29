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
        "serializable_register",
        "Serializable",
        "serialize_object",
        "deserialize_object",
    )

from object import Object
import dbus
import gobject

__serializable_name_dict = dict()

def serializable_register(classobj):
    if not issubclass(classobj, Serializable):
        raise "%s is not a sub-class of Serializable" % str(classobj)
    __serializable_name_dict[classobj.__NAME__] = classobj

def serialize_object(o):
    if isinstance(o, Serializable):
        l = [o.__NAME__]
        o.serialize(l)
        return dbus.Struct(l)
    else:
        return o

def deserialize_object(v):
    if isinstance(v, tuple):
        struct = list(v)
        type_name = struct.pop(0)
        type_class = __serializable_name_dict[type_name]
        o = type_class()
        o.deserialize (struct)
        return o
    return v

class Serializable(Object):
    __gtype_name__ = "IBusSerializable"
    __NAME__ = "IBusSerializable"
    def __init__(self):
        super(Serializable, self).__init__()
        self.__attachments = dict()

    def serialize(self, struct):
        d = dbus.Dictionary(signature="sv")
        for k, v in self.__attachments.items():
            d[k] = serialize(v)
        struct.append(d)

    def deserialize(self, struct):
        d = struct.pop(0)
        self.__attachments = dict()
        for k, v in d.items():
            self.__atachments[k] = deserialize(v)

    def do_destroy(self):
        self.__attachments = None
        super(Serializable, self).do_destroy()

__serializable_name_dict["IBusSerializable"] = Serializable
