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

import object
import serializable
import interface

class EngineBase(object.Object):
    def __init__(self, bus, object_path):
        super(EngineBase, self).__init__()
        self.__proxy = EngineProxy (self, bus.get_dbusconn(), object_path)

    def process_key_event(self, keyval, keycode, state):
        return False

    def focus_in(self):
        pass

    def focus_out(self):
        pass

    def set_cursor_location(self, x, y, w, h):
        pass

    def set_capabilities(self, cap):
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

    def commit_text(self, text):
        text = serializable.serialize_object(text)
        return self.__proxy.CommitText(text)

    def forward_key_event(self, keyval, state):
        return self.__proxy.ForwardKeyEvent(keyval, state)

    def update_preedit_text(self, text, cursor_pos, visible):
        text = serializable.serialize_object(text)
        return self.__proxy.UpdatePreeditText(text, cursor_pos, visible)

    def show_preedit_text(self):
        return self.__proxy.ShowPreeditText()

    def hide_preedit_text(self):
        return self.__proxy.HidePreeditText()

    def update_auxiliary_text(self, text, visible):
        text = serializable.serialize_object(text)
        return self.__proxy.UpdateAuxiliaryText(text, visible)

    def show_auxiliary_text(self):
        return self.__proxy.ShowAuxiliaryText()

    def hide_auxiliary_text(self):
        return self.__proxy.HideAuxiliaryText()

    def update_lookup_table(self, lookup_table, visible, just_current_page = False):
        if just_current_page:
            lookup_table = lookup_table.get_current_page_as_lookup_table()
        dbus_values = serializable.serialize_object(lookup_table)
        return self.__proxy.UpdateLookupTable(dbus_values, visible)

    def show_lookup_table(self):
        return self.__proxy.ShowLookupTable()

    def hide_lookup_table(self):
        return self.__proxy.HideLookupTable()

    def page_up_lookup_table(self):
        return self.__proxy.PageUpLookupTable()

    def page_down_lookup_table(self):
        return self.__proxy.PageDownLookupTable()

    def cursor_up_lookup_table(self):
        return self.__proxy.CursorUpLookupTable()

    def cursor_down_lookup_table(self):
        return self.__proxy.CursorDownLookupTable()

    def register_properties(self, props):
        dbus_values = serializable.serialize_object(props)
        return self.__proxy.RegisterProperties(dbus_values)

    def update_property(self, prop):
        dbus_values = serializable.serialize_object(prop)
        return self.__proxy.UpdateProperty(dbus_values)

    def get_dbus_object(self):
        return self.__proxy

    def do_destroy(self):
        self.__proxy = None
        super(EngineBase,self).do_destroy()


class EngineProxy(interface.IEngine):
    def __init__(self, engine, conn, object_path):
        super(EngineProxy, self).__init__(conn, object_path)
        self.__engine = engine

    def ProcessKeyEvent(self, keyval, keycode, state):
        return self.__engine.process_key_event(keyval, keycode, state)

    def FocusIn(self):
        return self.__engine.focus_in()

    def FocusOut(self):
        return self.__engine.focus_out()

    def SetCursorLocation(self, x, y, w, h):
        return self.__engine.set_cursor_location(x, y, w, h)

    def SetCapabilities(self, caps):
        return self.__engine.set_capabilities(caps)

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

    def CandidateClicked(self, index, button, state):
        return self.__engine.candidate_clicked(index, button, state)

    def Enable(self):
        return self.__engine.enable()

    def Disable(self):
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

