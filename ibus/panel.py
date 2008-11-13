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
        "PanelBase",
        "PanelItem",
        "PanelButton",
        "PanelToggleButton",
        "PanelMenu",
        "IBUS_SERVICE_PANEL",
        "IBUS_PATH_PANEL"
    )

IBUS_SERVICE_PANEL = "org.freedesktop.IBus.Panel"
IBUS_PATH_PANEL = "/org/freedesktop/IBus/Panel"

import ibus
from ibus import interface

class PanelItem:
    pass

class PanelButton(PanelItem):
    pass

class PanelToggleButton(PanelButton):
    pass

class PanelMenu(PanelItem):
    pass

class PanelBase(ibus.Object):
    def __init__(self, bus):
        super(PanelBase, self).__init__()
        self.__proxy = PanelProxy(self, bus.get_dbusconn())

    def set_cursor_location(self, x, y, w, h):
        pass

    def update_preedit(self, text, attrs, cursor_pos, visible):
        pass

    def show_preedit(self):
        pass

    def hide_preedit(self):
        pass

    def update_aux_string(self, text, attrs, visible):
        pass

    def show_aux_string(self):
        pass

    def hide_aux_string(self):
        pass

    def update_lookup_table(self, lookup_table, visible):
        pass

    def show_lookup_table(self):
        pass

    def hide_lookup_table(self):
        pass

    def show_candidate_window(self):
        pass

    def page_up_lookup_table(self):
        pass

    def page_down_lookup_table(self):
        pass

    def cursor_up_lookup_table(self):
        pass

    def cursor_down_lookup_table(self):
        pass

    def hide_candidate_window(self):
        pass

    def show_language_bar(self):
        pass

    def hide_language_bar(self):
        pass

    def register_properties(self, props):
        pass

    def update_property(self, prop):
        pass

    def focus_in(self, ic):
        pass

    def focus_out(self, ic):
        pass

    def states_changed(self):
        pass

    def reset(self):
        pass

    def start_setup(self):
        pass

    def page_up(self):
        self.__proxy.PageUp()

    def page_down(self):
        self.__proxy.PageDown()

    def cursor_up(self):
        self.__proxy.CursorUp()

    def cursor_down(self):
        self.__proxy.CursorDown()

    def property_activate(self, prop_name, prop_state):
        self.__proxy.PropertyActivate(prop_name, prop_state)

    def property_show(self, prop_name):
        self.__proxy.PropertyShow(prop_name)

    def property_hide(self, prop_name):
        self.__proxy.PropertyHide(prop_name)


class PanelProxy(interface.IPanel):
    def __init__ (self, panel, dbusconn):
        super(PanelProxy, self).__init__(dbusconn, IBUS_PATH_PANEL)
        self.__dbusconn = dbusconn
        self.__panel = panel

    def SetCursorLocation(self, x, y, w, h):
        self.__panel.set_cursor_location(x, y, w, h)

    def UpdatePreedit(self, text, attrs, cursor_pos, show):
        attrs = ibus.attr_list_from_dbus_value(attrs)
        self.__panel.update_preedit(text, attrs, cursor_pos, show)

    def ShowPreedit(self):
        self.__panel.show_preedit()

    def HidePreedit(self):
        self.__panel.hide_preedit()

    def UpdateAuxString(self, text, attrs, show):
        attrs = ibus.attr_list_from_dbus_value(attrs)
        self.__panel.update_aux_string(text, attrs, show)

    def ShowAuxString(self):
        self.__panel.show_aux_string()

    def HideAuxString(self):
        self.__panel.hide_aux_string()

    def UpdateLookupTable(self, lookup_table, show):
        lookup_table = ibus.lookup_table_from_dbus_value(lookup_table)
        self.__panel.update_lookup_table(lookup_table, show)

    def ShowLookupTable(self):
        self.__panel.show_lookup_table()

    def HideLookupTable(self):
        self.__panel.hide_lookup_table()

    def PageUpLookupTable(self):
        self.__panel.page_up_lookup_table()

    def PageDownLookupTable(self):
        self.__panel.page_down_lookup_table()

    def CursorUpLookupTable(self):
        self.__panel.cursor_up_lookup_table()

    def CursorDownLookupTable(self):
        self.__panel.cursor_down_lookup_table()

    def ShowCandidateWindow(self):
        self.__panel.show_candidate_window()

    def HideCandidateWindow(self):
        self.__panel.hide_candidate_window()

    def ShowLanguageBar(self):
        self.__panel.show_language_bar()

    def HideLanguageBar(self):
        self.__panel.hide_language_bar()

    def RegisterProperties(self, props):
        props = ibus.prop_list_from_dbus_value(props)
        self.__panel.register_properties(props)

    def UpdateProperty(self, prop):
        prop = ibus.property_from_dbus_value(prop)
        self.__panel.update_property(prop)

    def FocusIn(self, ic):
        self.__panel.focus_in(ic)

    def FocusOut(self, ic):
        self.__panel.focus_out(ic)

    def StatesChanged(self):
        self.__panel.states_changed()

    def Reset(self):
        self.__panel.reset()

    def StartSetup(self):
        self.__panel.start_setup()

    def Destroy(self):
        self.__panel.destroy()

