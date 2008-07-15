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

class Property(object):
	def __init__(self, name,
						type = PROP_TYPE_NORMAL,
						label = "",
						icon = "",
						tooltip = "",
						sensitive = True,
						visible = True,
						state = PROP_STATE_UNCHECKED):
		super(Property, self).__init__()
		self._name = name
		self._type = type
		self._label = label
		self._icon = icon
		self._tooltip = tooltip
		self._sensitive = sensitive
		self._visible = visible
		self._state = state
		self._sub_props = PropList()

	def set_sub_props(self, props):
		self._sub_props = props

	def get_sub_props(self):
		return self._sub_props

	def get_name(self):
		return self._name

	def get_type(self):
		return self._type

	def set_label(self, label):
		self._label = label

	def get_label(self):
		return self._label

	def set_tooltip(self, tooltip):
		self._tooltip = tooltip

	def get_tooltip(self):
		return self._tooltip

	def set_state(self, state):
		self._state = state

	def get_state(self):
		return self._state

	def set_sensitive(self, sensitive):
		self._sensitive = sensitive

	def get_sensitive(self):
		return self._sensitive

	def set_visible(self, visible):
		self._visible = visible

	def get_visible(self):
		return self._visible

	def is_same(self, prop, test_all = True):
		if self._name != prop._name or self._type != prop._type:
			return False
		if not test_all:
			return True
		if self._label != prop._label or \
			self._icon != prop._icon or \
			self._tooltip != prop._tooltip or \
			self._sensitive != prop._sensitive or \
			self._visible != prop._visible or \
			self._state != prop._state:
			return False
		return self._sub_props.is_same(prop._sub_props, test_all)


	def to_dbus_value(self):
		sub_props = self._sub_props.to_dbus_value()
		values = (dbus.String(self._name),
				dbus.Int32(self._type),
				dbus.String(self._label),
				dbus.String(self._icon),
				dbus.String(self._tooltip),
				dbus.Boolean(self._sensitive),
				dbus.Boolean(self._visible),
				dbus.Int32(self._state),
				sub_props)
		return dbus.Struct(values)

	def from_dbus_value(self, value):
		self._name, \
		self._type, \
		self._label, \
		self._icon, \
		self._tooltip, \
		self._sensitive, \
		self._visible, \
		self._state, \
		props = value

		self._sub_props = prop_list_from_dbus_value(props)

def property_from_dbus_value(value):
	p = Property("")
	p.from_dbus_value(value)
	return p

class PropList(object):
	def __init__(self):
		super(PropList, self).__init__()
		self._props = []

	def append(self, prop):
		self._props.append(prop)

	def prepand(self, prop):
		self._props.insert(0, prop)

	def insert(self, index, prop):
		self._props.insert(index, prop)

	def get_properties(self):
		return self._props[:]

	def is_same(self, props, test_all = True):
		if len(props.get_properties()) != len(self.get_properties()):
			return False

		for a, b in zip(self.get_properties(), props.get_properties()):
			if not a.is_same(b, test_all):
				return False
		return False

	def to_dbus_value(self):
		props = map(lambda p: p.to_dbus_value(), self._props)
		return dbus.Array(props, signature = "v")

	def from_dbus_value(self, value):
		props = []
		for p in value:
			props.append(property_from_dbus_value(p))
		self._props = props

	def __iter__(self):
		return self._props.__iter__()

	def __getitem__(self, i):
		return self._props.__getitem__(i)

def prop_list_from_dbus_value(value):
	props = PropList()
	props.from_dbus_value(value)
	return props

def test():
	props = PropList()
	props.append(Property("a"))
	props.append(Property("b"))
	props.append(Property("c"))
	props.append(Property("d"))
	value = props.to_dbus_value()
	print prop_list_from_dbus_value(value)

	p = Property("z")
	p.set_sub_props(props)
	props = PropList()
	props.append(p)
	value = props.to_dbus_value()
	print prop_list_from_dbus_value(value)

if __name__ == "__main__":
	test()
