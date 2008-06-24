# vim:set noet ts=4:
# -*- coding: utf-8 -*-
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

import gobject
import gtk
import pango
import dbus
import ibus
import anthy
from tables import *
from ibus import keysyms
from ibus import interface

class Engine (interface.IEngine):
	def __init__ (self, dbusconn, object_path):
		interface.IEngine.__init__ (self, dbusconn, object_path)
		self._dbusconn = dbusconn

		# create anthy context
		self._context = anthy.anthy_context ()
		self._context._set_encoding (anthy.ANTHY_UTF8_ENCODING)

		self._lookup_table = ibus.LookupTable ()
		self._prop_list = self._init_props ()

		# use reset to init values
		self._reset ()

	def _init_props (self):
		props = ibus.PropList ()

		# init input mode properties
		mode_prop = ibus.Property (name = "InputMode",
							type = ibus.PROP_TYPE_MENU,
							label = "あ",
							tooltip = "Switch input mode")
		mode_props = ibus.PropList ()
		mode_props.append (ibus.Property (name = "InputModeHiragana",
										type = ibus.PROP_TYPE_RADIO,
										label = u"Hiragana"))
		mode_props.append (ibus.Property (name = "InputModeKatagana",
										type = ibus.PROP_TYPE_RADIO,
										label = u"Katagana"))
		mode_props.append (ibus.Property (name = "InputModeHalfWidthHiragana",
										type = ibus.PROP_TYPE_RADIO,
										label = u"Half width hiragana"))
		mode_props.append (ibus.Property (name = "InputModeHalfWidthKatagana",
										type = ibus.PROP_TYPE_RADIO,
										label = u"Half width katagana"))
		mode_props.append (ibus.Property (name = "InputModeLatin",
										type = ibus.PROP_TYPE_RADIO,
										label = u"Direct input"))
		mode_prop.set_sub_props (mode_props)
		props.append (mode_prop)


		# init test property
		test_prop = ibus.Property (name = "TestProp",
							type = ibus.PROP_TYPE_TOGGLE,
							label = "あ",
							tooltip = "test property")
		props.append (test_prop)

		return props

	# reset values of engine
	def _reset (self):
		self._input_chars = u""
		self._convert_chars = u""
		self._cursor_pos = 0
		self._need_update = False
		self._convert_begined = False
		self._segments = []
		self._lookup_table.clean ()
		self._lookup_table_visible = False

	# begine convert
	def _begin_convert (self):
		if self._convert_begined:
			return
		self._convert_begined = True

		self._context.set_string (self._input_chars.encode ("utf-8"))
		conv_stat = anthy.anthy_conv_stat ()
		self._context.get_stat (conv_stat)

		for i in xrange (0, conv_stat.nr_segment):
			buf = " " * 100
			l = self._context.get_segment (i, 0, buf, 100)
			text = unicode (buf[:l], "utf-8")
			self._segments.append ((0, text))

		self._cursor_pos = 0
		self._fill_lookup_table ()
		self._lookup_table_visible = False

	def _fill_lookup_table (self):
		# get segment stat
		seg_stat = anthy.anthy_segment_stat ()
		self._context.get_segment_stat (self._cursor_pos, seg_stat)

		# fill lookup_table
		self._lookup_table.clean ()
		for i in xrange (0, seg_stat.nr_candidate):
			buf = " " * 100
			l = self._context.get_segment (self._cursor_pos, i, buf, 100)
			candidate = unicode (buf[:l], "utf-8")
			self._lookup_table.append_candidate (candidate)


	def _invalidate (self):
		if self._need_update:
			return
		self._need_update = True
		gobject.idle_add (self._update, priority = gobject.PRIORITY_LOW)

	def _page_up (self):
		# only process cursor down in convert mode
		if not self._convert_begined:
			return False

		if not self._lookup_table.page_up ():
			return False

		candidate = self._lookup_table.get_current_candidate ()[0]
		index = self._lookup_table.get_cursor_pos ()
		self._segments[self._cursor_pos] = index, candidate
		self._invalidate ()
		return True

	def _page_down (self):
		# only process cursor down in convert mode
		if not self._convert_begined:
			return False

		if not self._lookup_table.page_down ():
			return False

		candidate = self._lookup_table.get_current_candidate ()[0]
		index = self._lookup_table.get_cursor_pos ()
		self._segments[self._cursor_pos] = index, candidate
		self._invalidate ()
		return True

	def _cursor_up (self):
		# only process cursor down in convert mode
		if not self._convert_begined:
			return False

		if not self._lookup_table.cursor_up ():
			return False

		candidate = self._lookup_table.get_current_candidate ()[0]
		index = self._lookup_table.get_cursor_pos ()
		self._segments[self._cursor_pos] = index, candidate
		self._invalidate ()
		return True

	def _cursor_down (self):
		# only process cursor down in convert mode
		if not self._convert_begined:
			return False

		if not self._lookup_table.cursor_down ():
			return False

		candidate = self._lookup_table.get_current_candidate ()[0]
		index = self._lookup_table.get_cursor_pos ()
		self._segments[self._cursor_pos] = index, candidate
		self._invalidate ()
		return True

	def _commit_string (self, text):
		self._reset ()
		self.CommitString (text)
		self._invalidate ()

	def _update_input_chars (self):
		begin, end  = max (self._cursor_pos - 4, 0), self._cursor_pos

		for i in range (begin, end):
			text = self._input_chars[i:end]
			romja = romaji_typing_rule.get (text, None)
			if romja != None:
				self._input_chars = u"".join ((self._input_chars[:i], romja, self._input_chars[end:]))
				self._cursor_pos -= len(text)
				self._cursor_pos += len(romja)

		attrs = ibus.AttrList ()
		attrs.append (ibus.AttributeUnderline (pango.UNDERLINE_SINGLE, 0, len (self._input_chars.encode ("utf-8"))))

		self.UpdatePreedit (dbus.String (self._input_chars),
				attrs.to_dbus_value (),
				dbus.Int32 (self._cursor_pos),
				len (self._input_chars) > 0)
		self.UpdateAuxString (u"", ibus.AttrList ().to_dbus_value (), False)
		self.UpdateLookupTable (self._lookup_table.to_dbus_value (), self._lookup_table_visible)

	def _update_convert_chars (self):
		self._convert_chars = u""
		buf = " " * 100
		pos = 0
		i = 0
		for seg_index, text in self._segments:
			self._convert_chars += text
			if i <= self._cursor_pos:
				pos += len (text)
			i += 1

		attrs = ibus.AttrList ()
		attrs.append (ibus.AttributeUnderline (pango.UNDERLINE_SINGLE, 0, len (self._convert_chars)))
		attrs.append (ibus.AttributeBackground (ibus.RGB (200, 200, 240),
				pos - len (self._segments[self._cursor_pos][1]),
				pos))
		self.UpdatePreedit (dbus.String (self._convert_chars),
				attrs.to_dbus_value (),
				dbus.Int32 (pos),
				True)
		aux_string = u"( %d / %d )" % (self._lookup_table.get_cursor_pos () + 1, self._lookup_table.get_number_of_candidates())
		self.UpdateAuxString (aux_string, ibus.AttrList ().to_dbus_value (), self._lookup_table_visible)
		self.UpdateLookupTable (self._lookup_table.to_dbus_value (), self._lookup_table_visible)

	def _update (self):
		self._need_update = False
		if self._convert_begined == False:
			self._update_input_chars ()
		else:
			self._update_convert_chars ()

	def _on_key_return (self):
		if not self._input_chars:
			return False
		if self._convert_begined == False:
			self._commit_string (self._input_chars)
		else:
			i = 0
			for seg_index, text in self._segments:
				self._context.commit_segment (i, seg_index)
			self._commit_string (self._convert_chars)
		return True

	def _on_key_escape (self):
		if not self._input_chars:
			return False
		self._reset ()
		self._invalidate ()
		return True

	def _on_key_back_space (self):
		if not self._input_chars:
			return False

		if self._convert_begined:
			self._convert_begined = False
			self._cursor_pos = len (self._input_chars)
			self._lookup_table.clean ()
			self._lookup_table_visible = False
		elif self._cursor_pos > 0:
			self._input_chars = self._input_chars[:self._cursor_pos - 1] + self._input_chars [self._cursor_pos:]
			self._cursor_pos -= 1

		self._invalidate ()
		return True

	def _on_key_delete (self):
		if not self._input_chars:
			return False

		if self._convert_begined:
			self._convert_begined = False
			self._cursor_pos = len (self._input_chars)
			self._lookup_table.clean ()
			self._lookup_table_visible = False
		elif self._cursor_pos < len (self._input_chars):
			self._input_chars = self._input_chars[:self._cursor_pos] + self._input_chars [self._cursor_pos + 1:]

		self._invalidate ()
		return True

	def _on_key_space (self):
		if not self._input_chars:
			return False
		if self._convert_begined == False:
			self._begin_convert ()
			self._invalidate ()
		else:
			self._lookup_table_visible = True
			self._cursor_down ()
		return True

	def _on_key_up (self):
		if not self._input_chars:
			return False
		self._lookup_table_visible = True
		self._cursor_up ()
		return True

	def _on_key_down (self):
		if not self._input_chars:
			return False
		self._lookup_table_visible = True
		self._cursor_down ()
		return True

	def _on_key_page_up (self):
		if not self._input_chars:
			return False
		if self._lookup_table_visible == True:
			self._page_up ()
		return True

	def _on_key_page_down (self):
		if not self._input_chars:
			return False
		if self._lookup_table_visible == True:
			self._page_down ()
		return True

	def _on_key_left (self):
		if not self._input_chars:
			return False
		if self._cursor_pos == 0:
			return True
		self._cursor_pos -= 1
		self._lookup_table_visible = False
		self._fill_lookup_table ()
		self._invalidate ()
		return True

	def _on_key_right (self):
		if not self._input_chars:
			return False

		if self._convert_begined:
			max_pos = len (self._segments) - 1
		else:
			max_pos = len (self._input_chars)
		if self._cursor_pos == max_pos:
			return True
		self._cursor_pos += 1
		self._lookup_table_visible = False
		self._fill_lookup_table ()
		self._invalidate ()

		return True

	def _on_key_number (self, index):
		if not self._input_chars:
			return False

		if self._convert_begined and self._lookup_table_visible:
			candidates = self._lookup_table.get_canidates_in_current_page ()
			if self._lookup_table.set_cursor_pos_in_current_page (index):
				index = self._lookup_table.get_cursor_pos ()
				candidate = self._lookup_table.get_current_candidate ()[0]
				self._segments[self._cursor_pos] = index, candidate
				self._lookup_table_visible = False
				self._on_key_right ()
				self._invalidate ()
		return True


	def _on_key_common (self, keyval):
		if self._convert_begined:
			i = 0
			for seg_index, text in self._segments:
				self._context.commit_segment (i, seg_index)
			self._commit_string (self._convert_chars)
		self._input_chars += unichr (keyval)
		self._cursor_pos += 1
		self._invalidate ()
		return True

	def _process_key_event (self, keyval, is_press, state):
		# ignore key release events
		if not is_press:
			return False

		if keyval == keysyms.Return:
			return self._on_key_return ()
		elif keyval == keysyms.Escape:
			return self._on_key_escape ()
		elif keyval == keysyms.BackSpace:
			return self._on_key_back_space ()
		elif keyval == keysyms.Delete or keyval == keysyms.KP_Delete:
			return self._on_key_delete ()
		elif keyval == keysyms.space:
			return self._on_key_space ()
		elif keyval >= keysyms._1 and keyval <= keysyms._9:
			index = keyval - keysyms._1
			return self._on_key_number (index)
		elif keyval == keysyms.Page_Up or keyval == keysyms.KP_Page_Up:
			return self._on_key_page_up ()
		elif keyval == keysyms.Page_Down or keyval == keysyms.KP_Page_Down:
			return self._on_key_page_down ()
		elif keyval == keysyms.Up:
			return self._on_key_up ()
		elif keyval == keysyms.Down:
			return self._on_key_down ()
		elif keyval == keysyms.Left:
			return self._on_key_left ()
		elif keyval == keysyms.Right:
			return self._on_key_right ()
		elif keyval in xrange (keysyms.a, keysyms.z + 1) or \
			keyval in xrange (keysyms.A, keysyms.Z + 1):
			return self._on_key_common (keyval)
		else:
			return True

		return False

	# methods for dbus rpc
	def ProcessKeyEvent (self, keyval, is_press, state):
		try:
			return self._process_key_event (keyval, is_press, state)
		except Exception, e:
			print e
		return False

	def FocusIn (self):
		self.RegisterProperties (self._prop_list.to_dbus_value ())
		print "FocusIn"

	def FocusOut (self):
		print "FocusOut"

	def SetCursorLocation (self, x, y, w, h):
		pass

	def Reset (self):
		print "Reset"

	def PageUp (self):
		self._page_up ()

	def PageDown (self):
		self._page_down ()

	def CursorUp (self):
		self._cursor_up ()

	def CursorDown (self):
		self._cursor_down ()

	def SetEnable (self, enable):
		self._enable = enable
		if self._enable:
			self.RegisterProperties (self._prop_list.to_dbus_value ())

	def PropertyActivate (self, prop_name, prop_state):
		print "PropertyActivate (%s, %d)" % (prop_name, prop_state)

	def Destroy (self):
		print "Destroy"

