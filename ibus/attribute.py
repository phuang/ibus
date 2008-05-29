import dbus

ATTR_TYPE_UNDERLINE = 1
ATTR_TYPE_FOREGROUND = 2
ATTR_TYPE_BACKGROUND = 3

class Attribute:
	def __init__ (self, type, value, start_index, end_index):
		self._type = type
		self._value = value
		self._start_index = start_index
		self._end_index = end_index

	def to_dbus_value (self):
		values = [dbus.UInt32 (self._type),
				dbus.UInt32 (self._value),
				dbus.UInt32 (self._start_index),
				dbus.UInt32 (self._end_index)]
		return dbus.Array (values, signature="u")

	def from_dbus_value (self, value):
		if not isinstance (value, dbus.Array):
			raise dbus.Exception ("Attribute must be dbus.Array (uuuu)")

		if len (value) != 4 or not all (map (lambda x: isinstance (x, dbus.UInt32), value)):
			raise dbus.Exception ("Attribute must be dbus.Array (uuuu)")

		self._type = value[0]
		self._value = value[1]
		self._start_index = value[2]
		self._end_index = value[3]

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
	attrs = AttrList ()
	attrs.from_dbus_value (value)
	return attrs
