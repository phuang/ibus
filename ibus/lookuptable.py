import dbus
from attribute import *
from exception import *

class Candidates (list):

	def clear (self):
		del self[:]

	def to_dbus_value (self):
		value = dbus.Array ()
		for text, attrs in self:
			value.append (dbus.Struct ((dbus.String (text), attrs.to_dbus_value ())))
		return value

	def from_dbus_value (self, value):
		candidates = []
		if not isinstance (value, dbus.Array):
			raise dbus.Exception ("Candidates must from dbus.Array (a(sa(...))")
		for candidate in value:
			if not isinstance (candidate, dbus.Struct):
				raise IBusException ("Candidates must from dbus.Array (a(sa(...)))")
			if len (candidate) != 2 or \
				not isinstance (candidate[0], dbus.String):
				raise IBusException ("Candidates must from dbus.Array (a(sa(...)))")
			text = candidate[0]
			attrs = AttrList ()
			attrs.from_dbus_value (candidate[1])
			
			candidates.append ((text, attrs))
		self.clear ()
		self[:] = candidates
			

class LookupTable:
	def __init__ (self, page_size = 5):
		self._page_size = page_size
		self._cursor_visible = False
		self._cursor_pos = 0
		self._candidates = Candidates ()

	def set_page_size (self, page_size):
		self._page_size = page_size

	def get_page_size (self):
		return self._page_size

	def show_cursor (self, show = True):
		self._cursor_visible = show
	
	def is_cursor_visible (self):
		return self._cursor_visible

	def get_current_page_start (self):
		return (self._cursor_pos / self._page_size) * self._page_size

	def set_cursor_pos (self, pos):
		self._current_pos = pos

	def get_cursor_pos (self):
		return self._current_pos

	def get_cursor_pos_in_current_page (self):
		return self._current_pos % self._page_size

	def page_up (self):
		pass

	def page_down (self):
		pass

	def cursor_up (self):
		pass

	def cursor_down (self):
		pass

	def clear (self):
		self._candidates.clear ()

	def append_candidate (self, candidate, attrs = None):
		if attrs == None:
			attrs = AttrList ()
		self._candidates.append ((candidate, attrs))

	def get_candidate (self, index):
		return self._candidates[index]

	def to_dbus_struct (self):
		value = (dbus.UInt32 (self._page_size),
				 dbus.UInt32 (self._cursor_pos),
				 dbus.Boolean (self._cursor_visible),
				 self._candidates.to_dbus_value ())
		return dbus.Struct (value)

	def from_dbus_struct (self, value):
		if not isinstance (value, dbus.Struct):
			raise dbus.Exception ("LookupTable must from dbus.Struct (uuba(...))")

		if len (value) != 4 or \
			not isinstance (value[0], dbus.UInt32) or \
			not isinstance (value[1], dbus.UInt32) or \
			not isinstance (value[2], dbus.Boolean):
			raise dbus.Exception ("LookupTable must from dbus.Struct (uuba(...))")

		self._candidates.from_dbus_value (value[3])
		self._page_size = value[0]
		self._cursor_pos = value[1]
		self._cursor_visible = value[2]


def test ():
	t = LookupTable ()
	attrs = AttrList ()
	attrs.append (AttributeBackground (RGB (233, 0,1), 0, 3))
	t.append_candidate ("Hello", attrs)
	value = t.to_dbus_struct ()
	print value
	t.from_dbus_struct (value)

if __name__ == "__main__":
	test ()
