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

import gobject
import ibus

IBUS_CAP_PREEDIT = 1
IBUS_CAP_AUX_STRING = 1 << 1
IBUS_CAP_LOOKUP_TABLE = 1 << 2
IBUS_CAP_FOCUS = 1 << 3

class InputContext(ibus.Object):
    id = 1
    __gsignals__ = {
        "update-preedit" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_PYOBJECT, gobject.TYPE_PYOBJECT, gobject.TYPE_INT, gobject.TYPE_BOOLEAN)),
        "show-preedit" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
        "hide-preedit" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
        "update-aux-string" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_PYOBJECT, gobject.TYPE_PYOBJECT, gobject.TYPE_BOOLEAN)),
        "show-aux-string" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
        "hide-aux-string" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
        "update-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_PYOBJECT, gobject.TYPE_BOOLEAN)),
        "show-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
        "hide-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
        "page-up-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
        "page-down-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
        "cursor-up-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
        "cursor-down-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
        "register-properties" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_PYOBJECT, )),
        "update-property" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_PYOBJECT, )),
        "engine-lost" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()),
    }

    def __init__(self, name, ibusconn):
        super(InputContext, self).__init__()
        self.__ibusconn = ibusconn
        self.__ibusconn.connect("destroy", self.__ibusconn_destroy_cb)
        self.__path = ibus.IBUS_PATH_IBUS + "/" + str(InputContext.id)
        InputContext.id += 1
        self.__proxy = InputContextProxy(self, ibusconn, self.__path)

        # init default values
        self.__enable = False
        self.__engine = None
        self.__engine_handlers = []

        # client state
        self.__aux_string = None
        self.__aux_attrs = None
        self.__aux_visible = False

        # capabitlies
        self.__support_preedit = True
        self.__support_aux_string = False
        self.__support_lookup_table = False
        self.__support_focus = True

        self.__preedit_string = None
        self.__preedit_attrs = None
        self.__cursor_pos = 0
        self.__preedit_visible = False

        self.__lookup_table = None
        self.__lookup_table_visible = False

    def get_path(self):
        return self.__path;

    def get_preedit_string(self):
        return self.__preedit_string, self.__preedit_attrs, self.__cursor_pos

    def get_aux_string(self):
        return self.__aux_string, self.__aux_attrs

    def process_key_event(self, keyval, is_press, state,
                                reply_cb, error_cb):
        if self.__engine != None and self.__enable:
            self.__engine.process_key_event(keyval, is_press, state,
                                reply_cb, error_cb)
        else:
            reply_cb(False)

    def set_cursor_location(self, x, y, w, h):
        if self.__engine:
            self.__engine.set_cursor_location(x, y, w, h)

    def focus_in(self):
        if self.__engine and self.__enable:
            self.__engine.focus_in()

    def focus_out(self):
        if self.__engine and self.__enable:
            self.__engine.focus_out()

    def reset(self):
        if self.__engine and self.__enable:
            self.__engine.reset()

    def page_up(self):
        if self.__engine and self.__enable:
            self.__engine.page_up()

    def page_down(self):
        if self.__engine and self.__enable:
            self.__engine.page_down()

    def cursor_up(self):
        if self.__engine and self.__enable:
            self.__engine.cursor_up()

    def cursor_down(self):
        if self.__engine and self.__enable:
            self.__engine.cursor_down()

    def property_activate(self, prop_name, prop_state):
        if self.__engine and self.__enable:
            self.__engine.property_activate(prop_name, prop_state)

    def property_show(self, prop_name):
        if self.__engine and self.__enable:
            self.__engine.property_show(prop_name)

    def property_hide(self, prop_name):
        if self.__engine and self.__enable:
            self.__engine.property_hide(prop_name)

    def is_enabled(self):
        return self.__enable

    def set_capabilities(self, caps):
        self.__support_preedit = bool(caps & IBUS_CAP_PREEDIT)
        self.__support_aux_string = bool(caps & IBUS_CAP_AUX_STRING)
        self.__support_lookup_table = bool(caps & IBUS_CAP_LOOKUP_TABLE)
        self.__support_focus = bool(caps & IBUS_CAP_FOCUS)

    def get_support_focus(self):
        return self.__support_focus

    def set_enable(self, enable):
        if self.__enable != enable:
            self.__enable = enable
            if self.__enable:
                self.__ibusconn.emit_dbus_signal("Enabled", self.__id)
                if self.__engine:
                    self.__engine.enable()
                    self.__engine.focus_in()
            else:
                self.hide_preedit()
                self.__ibusconn.emit_dbus_signal("Disabled", self.__id)
                if self.__engine:
                    self.__engine.disable()

    def commit_string(self, text):
        if not self.__enable:
            return

        self.__proxy.CommitString(text)

    def update_preedit(self, text, attrs, cursor_pos, visible):
        if not self.__enable:
            return

        self.__preedit_string = text
        self.__preedit_attrs = attrs
        self.__cursor_pos = cursor_pos
        self.__preedit_visible = visible
        if self.__support_preedit:
            self.__proxy.UpdatePreedit(text, attrs, cursor_pos, visible)
        else:
            # show preedit on panel
            self.emit("update-preedit", text, attrs, cursor_pos, visible)

    def show_preedit(self):
        if not self.__enable:
            return

        self.__preedit_visible = True
        if self.__support_preedit:
            self.__proxy.ShowPreedit()
        else:
            # show preedit on panel
            self.emit("show-preedit")
    
    def hide_preedit(self):
        if not self.__enable:
            return

        self.__preedit_visible = False
        if self.__support_preedit:
            self.__proxy.HidePreedit()
        else:
            # show preedit on panel
            self.emit("show-preedit")
   
    def update_aux_string(self, text, attrs, visible):
        if not self.__enable:
            return

        self.__aux_string = text
        self.__aux_attrs = attrs
        self.__aux_visible = visible
        if self.__support_aux_string:
            self.__proxy.UpdateAuxString(text, attrs, visible)
        else:
            # show aux string on panel
            self.emit("update-aux-string", text, attrs, visible)

    def show_aux_string(self):
        if not self.__enable:
            return

        self.__aux_string_visible = True
        if self.__support_aux_string:
            self.__proxy.ShowAuxString()
        else:
            # show aux string on panel
            self.emit("show-aux-string")

    def hide_aux_string(self):
        if not self.__enable:
            return

        self.__aux_string_visible = False
        if self.__support_aux_string:
            self.__proxy.HideAuxString()
        else:
            # show aux string on panel
            self.emit("hide-aux-string")
    
    def update_lookup_table(self, table, visible):
        if not self.__enable:
            return

        if self.__support_lookup_table:
            self.__proxy.UpdateLookupTable(table, visible)
        else:
            # show lookup table on panel
            self.emit("update-lookup-table", table, visible)

    def show_lookup_table(self):
        if not self.__enable:
            return

        if self.__support_lookup_table:
            self.__proxy.ShowLookupTable()
        else:
            # show lookup table on panel
            self.emit("show-lookup-table")

    def hide_lookup_table(self):
        if not self.__enable:
            return

        if self.__support_lookup_table:
            self.__proxy.HideLookupTable()
        else:
            # show aux string on panel
            self.emit("hide-lookup-table")

    def set_engine(self, engine):
        if self.__engine == engine:
            return

        if self.__engine != None:
            self.__remove_engine_handlers()
            self.__engine.destroy()
            self.__engine = None

        self.__engine = engine
        self.__install_engine_handlers()

    def get_engine(self):
        return self.__engine

    def get_factory(self):
        if self.__engine:
            return self.__engine.get_factory()
        return None

    def __engine_destroy_cb(self, engine):
        if self.__engine == engine:
            self.__remove_engine_handlers()
        self.__engine = None
        self.__enable = False
        self.update_preedit (u"", ibus.AttrList().to_dbus_value(), 0 , False)
        self.__proxy.Disabled()
        self.emit("engine-lost")

    def __ibusconn_destroy_cb(self, ibusconn):
        if self.__engine != None:
            self.__remove_engine_handlers()
            self.__engine.destroy()
            self.__engine = None
        self.destroy()

    def page_up_lookup_table(self):
        if not self.__enable:
            return

        if self.__support_lookup_table:
            self.__proxy.PageUpLookupTable()
        else:
            self.emit("page-up-lookup-table")

    def page_down_lookup_table(self):
        if not self.__enable:
            return

        if self.__support_lookup_table:
            self.__proxy.PageDownLookupTable()
        else:
            self.emit("page-down-lookup-table")

    def cursor_up_lookup_table(self):
        if not self.__enable:
            return

        if self.__support_lookup_table:
            self.__proxy.CursorUpLookupTable()
        else:
            self.emit("cursor-up-lookup-table")

    def cursor_down_lookup_table(self):
        if not self.__enable:
            return

        if self.__support_lookup_table:
            self.__proxy.CursorDownLookupTable()
        else:
            self.emit("cursor-down-lookup-table")

    def register_properties(self, props):
        if not self.__enable:
            return
        self.emit("register-properties", props)

    def update_property(self, prop):
        if not self.__enable:
            return
        self.emit("update-property", prop)

    def __remove_engine_handlers(self):
        assert self.__engine != None

        map(self.__engine.disconnect, self.__engine_handlers)
        del self.__engine_handlers[:]

    def __install_engine_handlers(self):
        signals = (
            ("destroy",                     self.__engine_destroy_cb),
            ("commit-string",               lambda e, t: self.commit_string(t)),
            ("update-preedit",              lambda e, t, a, c, v: self.update_preedit(t, a, c, v)),
            ("show-preedit",                lambda e: self.show_preedit()),
            ("hide-preedit",                lambda e: self.hide_preedit()),
            ("update-aux-string",           lambda e, t, a, v: self.update_aux_string(t, a, v)),
            ("show-aux-string",             lambda e: self.show_aux_string()),
            ("hide-aux-string",             lambda e: self.hide_aux_string()),
            ("update-lookup-table",         lambda e, t, v: self.update_lookup_table(t, v)),
            ("show-lookup-table",           lambda e: self.show_lookup_table()),
            ("hide-lookup-table",           lambda e: self.hide_lookup_table()),
            ("page-up-lookup-table",        lambda e: self.page_up_lookup_table()),
            ("page-down-lookup-table",      lambda e: self.page_down_lookup_table()),
            ("cursor-up-lookup-table",      lambda e: self.cursor_up_lookup_table()),
            ("cursor-down-lookup-table",    lambda e: self.cursor_down_lookup_table()),
            ("register-properties",         lambda e, p: self.register_properties(p)),
            ("update-property",             lambda e, p: self.update_property(p))
        )

        for signal, handler in signals:
            id = self.__engine.connect(signal, handler)
            self.__engine_handlers.append(id)

gobject.type_register(InputContext)

class InputContextProxy(ibus.IInputContext):
    def __init__(self, context, conn, path):
        super(InputContextProxy, self).__init__(conn.get_dbusconn(), path)
        self.__context = context
        self.__conn = conn
        self.__conn.connect("destroy", self.__conn_destroy_cb)

    def __conn_destroy_cb(self, conn):
        self.__context = None
        self.__conn = None

    def ProcessKeyEvent(self, keyval, is_press, state, reply_cb, error_cb):
        return self.__context.process_key_event(keyval, is_press, state, reply_cb, error_cb)

    def SetCursorLocation(self, x, y, w, h):
        return self.__context.set_cursor_location(x, y, w, h)

    def FocusIn(self):
        return self.__context.focus_in()

    def FocusOut(self):
        return self.__context.focus_out()

    def Reset(self):
        return self.__context.reset()

    def IsEnabled(self):
        return self.__context.is_enabled()

    def SetCapabilities(self, caps):
        return self.__context.set_capabilities(caps)

    def GetInputContextStates(self):
        return self.__context.get_states()

    def Destroy(self):
        self.__context.destroy()
        self.__context = None
        self.__conn = None

