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
        "ATTR_TYPE_UNDERLINE",
        "ATTR_TYPE_FOREGROUND",
        "ATTR_TYPE_BACKGROUND",
        "ATTR_UNDERLINE_NONE",
        "ATTR_UNDERLINE_SINGLE",
        "ATTR_UNDERLINE_DOUBLE",
        "ATTR_UNDERLINE_LOW",
        "Attribute",
        "AttributeUnderline",
        "AttributeForeground",
        "AttributeBackground",
        "AttrList",
        "attribute_from_dbus_value",
        "attr_list_from_dbus_value",
        "ARGB", "RGB"
    )

import dbus

ATTR_TYPE_UNDERLINE = 1
ATTR_TYPE_FOREGROUND = 2
ATTR_TYPE_BACKGROUND = 3

ATTR_UNDERLINE_NONE = 0
ATTR_UNDERLINE_SINGLE = 1
ATTR_UNDERLINE_DOUBLE = 2
ATTR_UNDERLINE_LOW = 3

class Attribute:
    def __init__ (self, type, value, start_index, end_index):
        self.__type = type
        self.__value = value
        self.__start_index = start_index
        self.__end_index = end_index

    def get_type(self):
        return self.__type

    def get_value(self):
        return self.__value

    def get_start_index(self):
        return self.__start_index

    def get_end_index(self):
        return self.__end_index

    type        = property(get_type)
    value       = property(get_value)
    start_index = property(get_start_index)
    end_index   = property(get_end_index)

    def to_dbus_value (self):
        values = [dbus.UInt32 (self.__type),
                dbus.UInt32 (self.__value),
                dbus.UInt32 (self.__start_index),
                dbus.UInt32 (self.__end_index)]
        return dbus.Struct (values, signature="uuuu")

    def from_dbus_value (self, value):
        if not isinstance (value, dbus.Struct):
            raise dbus.Exception ("Attribute must be dbus.Struct uuuu")

        if len (value) != 4 or not all (map (lambda x: isinstance (x, dbus.UInt32), value)):
            raise dbus.Exception ("Attribute must be dbus.Struct uuuu")

        self.__type = value[0]
        self.__value = value[1]
        self.__start_index = value[2]
        self.__end_index = value[3]

def attribute_from_dbus_value (value):
    attribute = Attribute (0, 0, 0, 0)
    attribute.from_dbus_value (value)
    return attribute
        
class AttributeUnderline (Attribute):
    def __init__(self, value, start_index, end_index):
        Attribute.__init__ (self, ATTR_TYPE_UNDERLINE, value, start_index, end_index)

class AttributeForeground (Attribute):
    def __init__(self, value, start_index, end_index):
        Attribute.__init__ (self, ATTR_TYPE_FOREGROUND, value, start_index, end_index)

class AttributeBackground (Attribute):
    def __init__(self, value, start_index, end_index):
        Attribute.__init__ (self, ATTR_TYPE_BACKGROUND, value, start_index, end_index)

def ARGB (a, r, g, b):
    return ((a & 0xff)<<24) + ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff)

def RGB (r, g, b):
    return ARGB (255, r, g, b)

class AttrList:
    def __init__ (self, attrs = []):
        self._attrs = []
        for attr in attrs:
            self.append (attr)

    def append (self, attr):
        assert isinstance (attr, Attribute)
        self._attrs.append (attr)

    def to_dbus_value (self):
        array = dbus.Array (signature = "v")
        for attr in self._attrs:
            array.append (attr.to_dbus_value ())
        return array

    def from_dbus_value (self, value):
        attrs = []
        if not isinstance (value, dbus.Array):
            raise IBusException ("AttrList must from dbus.Array (uuuu)")

        for v in value:
            attr = attribute_from_dbus_value (v)
            attrs.append (attr)

        self._attrs = attrs

    def __iter__ (self):
        return self._attrs.__iter__ ()

def attr_list_from_dbus_value (value):
    if len(value) == 0:
        return None
    attrs = AttrList ()
    attrs.from_dbus_value (value)
    return attrs
