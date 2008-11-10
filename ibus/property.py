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
        "PROP_TYPE_NORMAL",
        "PROP_TYPE_TOGGLE",
        "PROP_TYPE_RADIO",
        "PROP_TYPE_SEPARATOR",
        "PROP_TYPE_MENU",
        "PROP_STATE_UNCHECKED",
        "PROP_STATE_CHECKED",
        "PROP_STATE_INCONSISTENT",
        "Property",
        "PropList",
        "property_from_dbus_value",
        "prop_list_from_dbus_value",
    )

import dbus

PROP_TYPE_NORMAL = 0
PROP_TYPE_TOGGLE = 1
PROP_TYPE_RADIO = 2
PROP_TYPE_MENU = 3
PROP_TYPE_SEPARATOR = 4

PROP_STATE_UNCHECKED = 0
PROP_STATE_CHECKED = 1
PROP_STATE_INCONSISTENT = 2

def _to_unicode(text):
    if isinstance(text, unicode):
        return text
    if isinstance(text, str):
        return unicode(text, "utf8")
    raise TypeError("text must be instance of unicode or str")

class Property(object):
    def __init__(self, name,
                        type = PROP_TYPE_NORMAL,
                        label = u"",
                        icon = u"",
                        tooltip = u"",
                        sensitive = True,
                        visible = True,
                        state = PROP_STATE_UNCHECKED):
        super(Property, self).__init__()
        self.__name = _to_unicode(name)
        self.__type = type
        self.label = label
        self.icon = icon
        self.tooltip = tooltip
        self.sensitive = sensitive
        self.visible = visible
        self.state = state
        self.sub_props = PropList()

    def set_sub_props(self, props):
        self.__sub_props = props

    def get_sub_props(self):
        return self.__sub_props

    def get_name(self):
        return self.__name

    def get_type(self):
        return self.__type

    def set_label(self, label):
        self.__label = _to_unicode(label)

    def get_label(self):
        return self.__label

    def set_icon(self, icon):
        self.__icon = _to_unicode(icon)

    def get_icon(self):
        return self.__icon

    def set_tooltip(self, tooltip):
        self.__tooltip = _to_unicode(tooltip)

    def get_tooltip(self):
        return self.__tooltip

    def set_state(self, state):
        self.__state = state

    def get_state(self):
        return self.__state

    def set_sensitive(self, sensitive):
        self.__sensitive = sensitive

    def get_sensitive(self):
        return self.__sensitive

    def set_visible(self, visible):
        self.__visible = visible

    def get_visible(self):
        return self.__visible

    name        = property(get_name)
    type        = property(get_type)
    label       = property(get_label, set_label)
    icon        = property(get_icon, set_icon)
    tooltip     = property(get_tooltip, set_tooltip)
    state       = property(get_state, set_state)
    sensitive   = property(get_sensitive, set_sensitive)
    visible     = property(get_visible, set_visible)
    sub_props   = property(get_sub_props, set_sub_props)

    def is_same(self, prop, test_all = True):
        if self.__name != prop.__name or self.__type != prop.__type:
            return False
        if not test_all:
            return True
        if self.__label != prop.__label or \
            self.__icon != prop.__icon or \
            self.__tooltip != prop.__tooltip or \
            self.__sensitive != prop.__sensitive or \
            self.__visible != prop.__visible or \
            self.__state != prop.__state:
            return False
        return self.__sub_props.is_same(prop.__sub_props, test_all)


    def to_dbus_value(self):
        sub_props = self.__sub_props.to_dbus_value()
        values = (dbus.String(self.__name),
                dbus.UInt32(self.__type),
                dbus.String(self.__label),
                dbus.String(self.__icon),
                dbus.String(self.__tooltip),
                dbus.Boolean(self.__sensitive),
                dbus.Boolean(self.__visible),
                dbus.UInt32(self.__state),
                sub_props)
        return dbus.Struct(values)

    def from_dbus_value(self, value):
        self.__name, \
        self.__type, \
        self.__label, \
        self.__icon, \
        self.__tooltip, \
        self.__sensitive, \
        self.__visible, \
        self.__state, \
        props = value

        self.__sub_props = prop_list_from_dbus_value(props)

def property_from_dbus_value(value):
    p = Property("")
    p.from_dbus_value(value)
    return p

class PropList(object):
    def __init__(self):
        super(PropList, self).__init__()
        self.__props = []

    def append(self, prop):
        self.__props.append(prop)

    def prepand(self, prop):
        self.__props.insert(0, prop)

    def insert(self, index, prop):
        self.__props.insert(index, prop)

    def get_properties(self):
        return self.__props[:]

    def is_same(self, props, test_all = True):
        if len(props.get_properties()) != len(self.get_properties()):
            return False

        for a, b in zip(self.get_properties(), props.get_properties()):
            if not a.is_same(b, test_all):
                return False
        return False

    def to_dbus_value(self):
        props = map(lambda p: p.to_dbus_value(), self.__props)
        return dbus.Array(props, signature = "(susssbbuv)")

    def from_dbus_value(self, value):
        props = []
        for p in value:
            props.append(property_from_dbus_value(p))
        self.__props = props

    def __iter__(self):
        return self.__props.__iter__()

    def __getitem__(self, i):
        return self.__props.__getitem__(i)

def prop_list_from_dbus_value(value):
    props = PropList()
    props.from_dbus_value(value)
    return props

def test():
    props = PropList()
    props.append(Property(u"a"))
    props.append(Property(u"b"))
    props.append(Property(u"c"))
    props.append(Property(u"d"))
    value = props.to_dbus_value()
    print prop_list_from_dbus_value(value)

    p = Property(u"z")
    p.set_sub_props(props)
    props = PropList()
    props.append(p)
    value = props.to_dbus_value()
    print prop_list_from_dbus_value(value)
    p.label = u"a"
    p.label = "a"
    p.label = 1

if __name__ == "__main__":
    test()
