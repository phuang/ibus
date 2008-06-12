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
		offsets.append (offset)
		for attr in attrs:
			pango_attr = None
			start_index = attrs._start_index if attrs._start_index >= 0 else 0
			end_index = attrs._end_index if attrs._end_index >= 0 else 0
			start_index = offsets[_start_index] if attrstart_index < len (offsets) else offsets[-1]
			end_index = offsets[_end_index] if end_index < len (offsets) else offsets[-1]
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
