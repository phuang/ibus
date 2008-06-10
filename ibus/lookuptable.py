import dbus
from attribute import *
from exception import *

class StringList (list):
	def clean (self):
		del self[:]

	def to_dbus_value (self):
		value = dbus.Array ([], signature="v")
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
			attrs = attr_list_from_dbus_value (candidate[1])
			candidates.append ((text, attrs))

		self.clean ()
		self[:] = candidates

class LookupTable:
	def __init__ (self, page_size = 5, labels = None):
		self._cursor_visible = False
		self._cursor_pos = 0
		self._candidates = StringList ()
		self.set_page_size (page_size)

	def set_page_size (self, page_size):
		self._page_size = page_size

	def get_page_size (self):
		return self._page_size

	def get_current_page_size (self):
		nr_candidate = len (self._candidates)
		nr_page, last_page_size = divmod (nr_candidate, self._page_size)
		if self._cursor_pos / self._page_size == nr_page:
			return last_page_size
		else:
			return self._page_size

	def set_labels (self, labels):
		self._labels == labels

	def show_cursor (self, show = True):
		self._cursor_visible = show

	def is_cursor_visible (self):
		return self._cursor_visible

	def get_current_page_start (self):
		return (self._cursor_pos / self._page_size) * self._page_size

	def set_cursor_pos (self, pos):
		self._cursor_pos = pos

	def get_cursor_pos (self):
		return self._cursor_pos

	def get_cursor_pos_in_current_page (self):
		page, pos_in_page = divmod (self._cursor_pos, self._page_size)
		return pos_in_page

	def page_up (self):
		if self._cursor_pos < self._page_size:
			return False

		self._cursor_pos -= self._page_size
		return True

	def page_down (self):
		current_page = self._cursor_pos / self._page_size
		nr_candidates = len (self._candidates)
		max_page = nr_candidates / self._page_size

		if current_page >= max_page:
			return False

		pos = self._cursor_pos + self._page_size
		if pos >= nr_candidates:
			return False
		self._cursor_pos = pos

		return True

	def cursor_up (self):
		if self._cursor_pos == 0:
			return False

		self._cursor_pos -= 1
		return True

	def cursor_down (self):
		if self._cursor_pos == len (self._candidates) - 1:
			return False

		self._cursor_pos += 1
		return True

	def clean (self):
		self._candidates.clean ()
		self._cursor_pos = 0

	def append_candidate (self, candidate, attrs = None):
		if attrs == None:
			attrs = AttrList ()
		self._candidates.append ((candidate, attrs))

	def get_candidate (self, index):
		return self._candidates[index]

	def get_canidates_in_current_page (self):
		page = self._cursor_pos / self._page_size
		start_index = page * self._page_size
		end_index = min ((page + 1) * self._page_size, len (self._candidates))
		return self._candidates[start_index:end_index]

	def get_current_candidate (self):
		return self._candidates [self._cursor_pos]

	def get_number_of_candidates (self):
		return len (self._candidates)

	def to_dbus_value (self):
		value = (dbus.Int32 (self._page_size),
				 dbus.Int32 (self._cursor_pos),
				 dbus.Boolean (self._cursor_visible),
				 self._candidates.to_dbus_value ())
		return dbus.Struct (value)

	def from_dbus_value (self, value):
		if not isinstance (value, dbus.Struct):
			raise dbus.Exception ("LookupTable must from dbus.Struct (uuba(...))")

		if len (value) != 4 or \
			not isinstance (value[0], dbus.Int32) or \
			not isinstance (value[1], dbus.Int32) or \
			not isinstance (value[2], dbus.Boolean):
			raise dbus.Exception ("LookupTable must from dbus.Struct (uuba(...))")

		self._candidates.from_dbus_value (value[3])
		self._page_size = value[0]
		self._cursor_pos = value[1]
		self._cursor_visible = value[2]

def lookup_table_from_dbus_value (value):
	lookup_table = LookupTable ()
	lookup_table.from_dbus_value (value)
	return lookup_table

def unit_test ():
	t = LookupTable ()
	# attrs = AttrList ()
	# attrs.append (AttributeBackground (RGB (233, 0,1), 0, 3))
	# attrs.append (AttributeUnderline (1, 3, 5))
	t.append_candidate ("Hello")
	value = t.to_dbus_value ()
	print value
	t = lookup_table_from_dbus_value (value)

if __name__ == "__main__":
	unit_test ()
