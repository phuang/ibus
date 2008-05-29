import pango
import ibus

class PangoAttrList (pango.AttrList):
	def __init__ (self, attrs):
		pango.AttrList.__init__ (self)
		if attrs == None:
			return
		for attr in attrs:
			pango_attr = None
			if attr._type == ibus.ATTR_TYPE_FOREGROUND:
				r = (attr._value & 0x00ff0000) >> 8
				g = (attr._value & 0x0000ff00)
				b = (attr._value & 0x000000ff) << 8
				pango_attr = pango.AttrForeground (r, g, b, 
					attr._start_index, attr._end_index)
			elif attr._type == ibus.ATTR_TYPE_BACKGROUND:
				r = (attr._value & 0x00ff0000) >> 8
				g = (attr._value & 0x0000ff00)
				b = (attr._value & 0x000000ff) << 8
				pango_attr = pango.AttrBackground (r, g, b, 
					attr._start_index, attr._end_index)
			elif attr._type == ibus.ATTR_TYPE_UNDERLINE:
				pango_attr = pango.AttrUnderline (attr._value,
										attr._start_index, attr._end_index)
			if pango_attr != None:
				self.insert (pango_attr)


if __name__ == "__main__":
	PangoAttrList (None)
