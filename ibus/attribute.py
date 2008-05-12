
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
		self._start_index = start_index
		self._end_index = end_index
		self._value = value

	def get_values (self):
		return [dbus.UInt32 (self._type), 
				dbus.UInt32 (self._value), 
				dbus.UInt32 (self._start_index), 
				dbus.UInt32 (self._end_index)]

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

	def get_array (self):
		get_values = lambda attr: attr.get_values ()
		return map (get_values, self._attrs)

