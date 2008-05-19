import dbus

ATTR_TYPE_DECORATION = 1
ATTR_TYPE_FOREGROUND = 2
ATTR_TYPE_BACKGROUND = 3

ATTR_DECORATION_NONE 		= 0
ATTR_DECORATION_UNDERLINE 	= 1
ATTR_DECORATION_HIGHLIGHT	= 2
ATTR_DECORATION_REVERSE		= 4

class Attribute:
	def __init__ (self, type, value, start_index, end_index):
		self._type = type
		self._value = value
		self._start_index = start_index
		self._end_index = end_index

	def to_dbus_value (self):
		values = (dbus.UInt32 (self._type),
				dbus.UInt32 (self._value),
				dbus.UInt32 (self._start_index),
				dbus.UInt32 (self._end_index))
		return dbus.Struct (values)

	def from_dbus_value (self, value):
		if not isinstance (value, dbus.Struct):
			raise dbus.Exception ("Attribute must be dbus.Struct (uuuu)")

		if len (value) != 4 or not all (map (lambda x: isinstance (x, dbus.UInt32), value)):
			raise dbus.Exception ("Attribute must be dbus.Struct (uuuu)")

		self._type = value[0]
		self._value = value[1]
		self._start_index = value[2]
		self._end_index = value[3]
		
		
class AttributeDecoration (Attribute):
	def __init__(self, value, start_index, end_index):
		Attribute.__init__ (self, ATTR_TYPE_DECORATION, value, start_index, end_index)


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
		array = dbus.Array ()
		for attr in self._attrs:
			array.append (attr.to_dbus_value ())
		return array

	def from_dbus_value (self, value):
		attrs = []
		if not isinstance (value, dbus.Array):
			raise IBusException ("AttrList must from dbus.Array (iiii)")

		for v in value:
			attr = Attribute (0, 0, 0, 0)
			attr.from_dbus_value (v)
			attrs.append (attr)

		self._attrs = attrs
