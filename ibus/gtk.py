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
			start_index = attr._start_index if attr._start_index >= 0 else 0
			end_index = attr._end_index if attr._end_index >= 0 else 0
			start_index = offsets[start_index] if start_index < len (offsets) else offsets[-1]
			end_index = offsets[end_index] if end_index < len (offsets) else offsets[-1]
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
