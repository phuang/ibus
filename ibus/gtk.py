import pango
import ibus

class PangoAttrList (pango.AttrList):
	def __init__ (self, attrs, unistr):
		pango.AttrList.__init__ (self)
		if attrs == None:
			return
		offsets = []
		offset = 0
		for c in unistr:
			offsets.append (offset)
			offset += len (c.encode ("utf8"))
		for attr in attrs:
			pango_attr = None
			start_index = offsets[attr._start_index]
			end_index = offsets[attr._end_index]
			if attr._type == ibus.ATTR_TYPE_FOREGROUND:
				r = (attr._value & 0x00ff0000) >> 8
				g = (attr._value & 0x0000ff00)
				b = (attr._value & 0x000000ff) << 8
				pango_attr = pango.AttrForeground (r, g, b, 
					start_index, end_index)
			elif attr._type == ibus.ATTR_TYPE_BACKGROUND:
				r = (attr._value & 0x00ff0000) >> 8
				g = (attr._value & 0x0000ff00)
				b = (attr._value & 0x000000ff) << 8
				pango_attr = pango.AttrBackground (r, g, b, 
					start_index, end_index)
			elif attr._type == ibus.ATTR_TYPE_UNDERLINE:
				pango_attr = pango.AttrUnderline (int (attr._value),
					start_index, end_index)
			if pango_attr != None:
				self.insert (pango_attr)


if __name__ == "__main__":
	PangoAttrList (None)
