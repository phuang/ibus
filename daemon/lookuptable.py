# vim:set et sts=4 sw=4:
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

import dbus

class Candidates(list):
    SIGNATURE = "a(saau)"
    def to_dbus_value(self):
        value = dbus.Array(signature = "(saau)")
        for text, attrs in self:
            value.append((text, attrs.to_dbus_value()), "(s%s)" % attrs.SIGNATURE)

    def from_dbus_value(self):
        pass

class LookupTable(object):
    SIGNATURE = "(ibia(saau))"

    def __init__(self, page_size = 5):
        self._page_size = page_size
        self._cursor_visible = False
        self._cursor_pos = 0
        self._candidates = []

    def set_page_size(self, page_size):
        self._page_size = page_size

    def get_page_size(self):
        return self._page_size

    def show_cursor(self):
        self._cursor_visible = True

    def hide_cursor(self):
        self._cursor_visible = False

    def is_cursor_visible(self):
        return self._cursor_visible

    def get_current_page_start(self):
        return(self._cursor_pos / self._page_size) * self._page_size

    def set_cursor_pos(self, pos):
        self._current_pos = pos

    def get_cursor_pos(self):
        return self._current_pos

    def get_cursor_pos_in_current_page(self):
        return self._current_pos % self._page_size

    def page_up(self):
        pass

    def page_down(self):
        pass

    def cursor_up(self):
        pass

    def cursor_down(self):
        pass

    def clear(self):
        self._candidates = []

    def append_candidate(self, candidate, attrs = None):
        self._candidates.append((candidates, attrs))

    def get_candidate(self, index):
        return self._candidates[index]

    def to_dbus_struct(self):
        pass

    def from_dbus_struct(self, value):
        pass
