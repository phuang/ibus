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

__all__ = (
        "LookupTable",
        "lookup_table_from_dbus_value"
    )

import dbus
from attribute import *
from exception import *

class StringList(list):
    def __init__(self, items = []):
        super(StringList, self).__init__(items)

    def clean(self):
        del self[:]

    def to_dbus_value(self):
        value = dbus.Array([], signature="v")
        for text, attrs in self:
            value.append(dbus.Struct((dbus.String(text), attrs.to_dbus_value())))
        return value

    def from_dbus_value(self, value):
        candidates = []
        if not isinstance(value, dbus.Array):
            raise dbus.Exception("Candidates must from dbus.Array(a(sa(...))")
        for candidate in value:
            if not isinstance(candidate, dbus.Struct):
                raise IBusException("Candidates must from dbus.Array(a(sa(...)))")
            if len(candidate) != 2 or \
                not isinstance(candidate[0], dbus.String):
                raise IBusException("Candidates must from dbus.Array(a(sa(...)))")
            text = candidate[0]
            attrs = attr_list_from_dbus_value(candidate[1])
            candidates.append((text, attrs))

        self.clean()
        self[:] = candidates

class LookupTable(object):
    def __init__(self, page_size = 5, labels = None):
        super(LookupTable, self).__init__()
        self.__cursor_visible = False
        self.__cursor_pos = 0
        self.__candidates = StringList()
        self.set_page_size(page_size)

    def set_page_size(self, page_size):
        self.__page_size = page_size

    def get_page_size(self):
        return self.__page_size

    def get_current_page_size(self):
        nr_candidate = len(self.__candidates)
        nr_page, last_page_size = divmod(nr_candidate, self.__page_size)
        if self.__cursor_pos / self.__page_size == nr_page:
            return last_page_size
        else:
            return self.__page_size

    def set_labels(self, labels):
        self.__labels == labels

    def show_cursor(self, show = True):
        self.__cursor_visible = show

    def is_cursor_visible(self):
        return self.__cursor_visible

    def get_current_page_start(self):
        return (self.__cursor_pos / self.__page_size) * self.__page_size

    def set_cursor_pos(self, pos):
        if pos >= len(self.__candidates) or pos < 0:
            return False
        self.__cursor_pos = pos
        return True

    def get_cursor_pos(self):
        return self.__cursor_pos

    def get_cursor_pos_in_current_page(self):
        page, pos_in_page = divmod(self.__cursor_pos, self.__page_size)
        return pos_in_page

    def set_cursor_pos_in_current_page(self, pos):
        if pos < 0 or pos >= self.__page_size:
            return False
        pos += self.get_current_page_start()
        if pos >= len(self.__candidates):
            return False
        self.__cursor_pos = pos
        return True


    def page_up(self):
        if self.__cursor_pos < self.__page_size:
            return False

        self.__cursor_pos -= self.__page_size
        return True

    def page_down(self):
        current_page = self.__cursor_pos / self.__page_size
        nr_candidates = len(self.__candidates)
        max_page = nr_candidates / self.__page_size

        if current_page >= max_page:
            return False

        pos = self.__cursor_pos + self.__page_size
        if pos >= nr_candidates:
            return False
        self.__cursor_pos = pos

        return True

    def cursor_up(self):
        if self.__cursor_pos == 0:
            return False

        self.__cursor_pos -= 1
        return True

    def cursor_down(self):
        if self.__cursor_pos == len(self.__candidates) - 1:
            return False

        self.__cursor_pos += 1
        return True

    def clean(self):
        self.__candidates.clean()
        self.__cursor_pos = 0

    def append_candidate(self, candidate, attrs = None):
        if attrs == None:
            attrs = AttrList()
        self.__candidates.append((candidate, attrs))

    def get_candidate(self, index):
        return self.__candidates[index]

    def get_canidates_in_current_page(self):
        page = self.__cursor_pos / self.__page_size
        start_index = page * self.__page_size
        end_index = min((page + 1) * self.__page_size, len(self.__candidates))
        return self.__candidates[start_index:end_index]

    def get_current_candidate(self):
        return self.__candidates [self.__cursor_pos]

    def get_number_of_candidates(self):
        return len(self.__candidates)

    def __len__(self):
        return self.get_number_of_candidates()

    def to_dbus_value(self):
        value = (dbus.Int32(self.__page_size),
                 dbus.Int32(self.__cursor_pos),
                 dbus.Boolean(self.__cursor_visible),
                 self.__candidates.to_dbus_value())
        return dbus.Struct(value)

    def current_page_to_dbus_value(self):
        candidates = self.get_canidates_in_current_page()
        candidates = StringList(candidates)
        value = (dbus.Int32(self.__page_size),
                 dbus.Int32(self.__cursor_pos % self.__page_size),
                 dbus.Boolean(self.__cursor_visible),
                 candidates.to_dbus_value())
        return dbus.Struct(value)

    def from_dbus_value(self, value):
        if not isinstance(value, dbus.Struct):
            raise dbus.Exception("LookupTable must from dbus.Struct(uuba(...))")

        if len(value) != 4 or \
            not isinstance(value[0], dbus.Int32) or \
            not isinstance(value[1], dbus.Int32) or \
            not isinstance(value[2], dbus.Boolean):
            raise dbus.Exception("LookupTable must from dbus.Struct(uuba(...))")

        self.__candidates.from_dbus_value(value[3])
        self.__page_size = value[0]
        self.__cursor_pos = value[1]
        self.__cursor_visible = value[2]

def lookup_table_from_dbus_value(value):
    lookup_table = LookupTable()
    lookup_table.from_dbus_value(value)
    return lookup_table

def unit_test():
    t = LookupTable()
    # attrs = AttrList()
    # attrs.append(AttributeBackground(RGB(233, 0,1), 0, 3))
    # attrs.append(AttributeUnderline(1, 3, 5))
    t.append_candidate("Hello")
    value = t.to_dbus_value()
    print value
    t = lookup_table_from_dbus_value(value)

if __name__ == "__main__":
    unit_test()
