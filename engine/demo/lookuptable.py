
class LookupTable:
	def __init__ (self, page_size = 5):
		self._page_size = page_size
		self._cursor_visibler = False
		self._cursor_pos = 0
		self._candidates = []

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
		self._candidates = []

	def append_candidate (self, candidate, attrs = None):
		self._candidates.append ((candidates, attrs))

	def get_candidate (self, index):
		return self._candidates[index]

