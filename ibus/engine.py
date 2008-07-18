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
        "EngineBase",
    )

import ibus
from ibus import interface

class EngineBase(ibus.Object):
    def __init__(self, conn, object_path):
        super(EngineBase, self).__init__()
        self.__proxy = EngineProxy (self, conn, object_path)

    def process_key_event(self, keyval, is_press, state):
        return False

    def focus_in(self):
        pass

    def focus_out(self):
        pass

    def set_cursor_location(self, x, y, w, h):
        pass

    def reset(self):
        pass

    def page_up(self):
        pass

    def page_down(self):
        pass

    def cursor_up(self):
        pass

    def cursor_down(self):
        pass

    def enable(self):
        pass

    def disable(self):
        pass

    def property_activate(self, prop_name, prop_state):
        pass

    def property_show(self, prop_name):
        pass

    def property_hide(self, prop_name):
        pass

    def commit_string(self, text):
        return self.__proxy.CommitString(text)

    def forward_key_event(self, keyval, is_press, state):
        return self.__proxy.ForwardKeyEvent(keyval, is_press, state)

    def update_preedit(self, text, attrs, cursor_pos, visible):
        return self.__proxy.UpdatePreedit(text, attrs.to_dbus_value(), cursor_pos, visible)

    def update_aux_string(self, text, attrs, visible):
        return self.__proxy.UpdateAuxString(text, attrs.to_dbus_value(), visible)

    def update_lookup_table(self, lookup_table, visible):
        return self.__proxy.UpdateLookupTable(lookup_table.to_dbus_value(), visible)

    def register_properties(self, props):
        return self.__proxy.RegisterProperties(props.to_dbus_value())

    def update_property(self, prop):
        return self.__proxy.UpdateProperty(prop.to_dbus_value())

    def get_dbus_object(self):
        return self.__proxy

    def do_destroy(self):
        self.__proxy = None
        super(EngineBase,self).do_destroy()


class EngineProxy(interface.IEngine):
    def __init__(self, engine, conn, object_path):
        super(EngineProxy, self).__init__(conn, object_path)
        self.__conn = conn
        self.__engine = engine

    def ProcessKeyEvent(self, keyval, is_press, state):
        return self.__engine.process_key_event(keyval, is_press, state)

    def FocusIn(self):
        return self.__engine.focus_in()

    def FocusOut(self):
        return self.__engine.focus_out()

    def SetCursorLocation(self, x, y, w, h):
        return self.__engine.set_cursor_location(x, y, w, h)

    def Reset(self):
        return self.__engine.reset()

    def PageUp(self):
        return self.__engine.page_up()

    def PageDown(self):
        return self.__engine.page_down()

    def CursorUp(self):
        return self.__engine.cursor_up()

    def CursorDown(self):
        return self.__engine.cursor_down()

    def Enable(self, enable):
        return self.__engine.enable()
    
    def Enable(self, enable):
        return self.__engine.disable()

    def PropertyActivate(self, prop_name, prop_state):
        return self.__engine.property_activate(prop_name, prop_state)

    def PropertyShow(self, prop_name):
        return self.__engine.property_show(prop_name)
    
    def PropertyHide(self, prop_name):
        return self.__engine.property_hide(prop_name)

    def Destroy(self):
        self.__engine.destroy()
        self.__engine = None
        self.remove_from_connection ()
        self.__conn = None

