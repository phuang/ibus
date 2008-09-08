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
        self.__id = "%d" % InputContext.id
        InputContext.id += 1
        self.__ibusconn = ibusconn
        self.__ibusconn.connect("destroy", self.__ibusconn_destroy_cb)

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

        self.__preedit_string = None
        self.__preedit_attrs = None
        self.__cursor_pos = 0
        self.__preedit_visible = False

        self.__lookup_table = None
        self.__lookup_table_visible = False

    def get_id(self):
        return self.__id;

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
        self.__support_preedit = (caps & (1 << 1)) != 0
        self.__support_aux_string = (caps & (1 << 1)) != 0
        self.__support_lookup_table = (caps & (1 << 2)) != 0

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
        self.__ibusconn.emit_dbus_signal("CommitString", self.__id, text)

    def update_preedit(self, text, attrs, cursor_pos, visible):
        self.__preedit_string = text
        self.__preedit_attrs = attrs
        self.__cursor_pos = cursor_pos
        self.__preedit_visible = visible
        if self.__support_preedit:
            self.__ibusconn.emit_dbus_signal("UpdatePreedit", self.__id, text, attrs, cursor_pos, visible)
        else:
            # show preedit on panel
            self.emit("update-preedit", text, attrs, cursor_pos, visible)

    def show_preedit(self):
        self.__preedit_visible = True
        if self.__support_preedit:
            self.__ibusconn.emit_dbus_signal("ShowPreedit", self.__id)
        else:
            # show preedit on panel
            self.emit("show-preedit")

    def hide_preedit(self):
        self.__preedit_visible = False
        if self.__support_preedit:
            self.__ibusconn.emit_dbus_signal("HidePreedit", self.__id)
        else:
            # show preedit on panel
            self.emit("hide-preedit")

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
        if self.__support_preedit:
            self.__ibusconn.emit_dbus_signal("UpdatePreedit",
                                self.__id,
                                u"",
                                ibus.AttrList().to_dbus_value(),
                                0,
                                False)
        self.__ibusconn.emit_dbus_signal("Disabled", self.__id)
        self.emit("engine-lost")

    def __ibusconn_destroy_cb(self, ibusconn):
        if self.__engine != None:
            self.__remove_engine_handlers()
            self.__engine.destroy()
            self.__engine = None
        self.destroy()

    def __commit_string_cb(self, engine, text):
        if not self.__enable:
            return
        self.commit_string(text)

    def __update_preedit_cb(self, engine, text, attrs, cursor_pos, visible):
        if not self.__enable:
            return
        self.update_preedit(text, attrs, cursor_pos, visible)

    def __show_preedit_cb(self, engine):
        if not self.__enable:
            return
        self.show_preedit()

    def __hide_preedit_cb(self, engine):
        if not self.__enable:
            return
        self.hide_preedit()

    def __update_aux_string_cb(self, engine, text, attrs, visible):
        if not self.__enable:
            return
        self.__aux_string = text
        self.__aux_attrs = attrs
        self.__aux_visible = visible

        if self.__support_aux_string:
            self.__ibusconn.emit_dbus_signal("UpdateAuxString", self.__id, text, attrs, visible)
        else:
            self.emit("update-aux-string", text, attrs, visible)

    def __show_aux_string_cb(self, engine):
        if not self.__enable:
            return
        self.__aux_visible = True

        if self.__support_aux_string:
            self.__ibusconn.emit_dbus_signal("ShowAuxString", self.__id)
        else:
            self.emit("show-aux-string")

    def __hide_aux_string_cb(self, engine):
        if not self.__enable:
            return
        self.__aux_visible = False

        if self.__support_aux_string:
            self.__ibusconn.emit_dbus_signal("HideAuxString", self.__id)
        else:
            self.emit("hide-aux-string")

    def __update_lookup_table_cb(self, engine, lookup_table, visible):
        if not self.__enable:
            return
        self.__lookup_table = lookup_table
        self.__lookup_table_visible = visible

        if self.__support_lookup_table:
            self.__ibusconn.emit_dbus_signal("UpdateLookupTable", self.__id, lookup_table, visible)
        else:
            self.emit("update-lookup-table", lookup_table, visible)

    def __show_lookup_table_cb(self, engine):
        if not self.__enable:
            return
        self.__lookup_table_visible = True

        if self.__support_lookup_table:
            self.__ibusconn.emit_dbus_signal("ShowLookupTable", self.__id)
        else:
            self.emit("show-lookup-table")

    def __hide_lookup_table_cb(self, engine):
        if not self.__enable:
            return
        self.__lookup_table_visible = False

        if self.__support_lookup_table:
            self.__ibusconn.emit_dbus_signal("HideLookupTable", self.__id)
        else:
            self.emit("hide-lookup-table")

    def __page_up_lookup_table_cb(self, engine):
        if not self.__enable:
            return

        if self.__support_lookup_table:
            self.__ibusconn.emit_dbus_signal("PageUpLookupTable", self.__id)
        else:
            self.emit("page-up-lookup-table")

    def __page_down_lookup_table_cb(self, engine):
        if not self.__enable:
            return

        if self.__support_lookup_table:
            self.__ibusconn.emit_dbus_signal("PageDownLookupTable", self.__id)
        else:
            self.emit("page-down-lookup-table")

    def __cursor_up_lookup_table_cb(self, engine):
        if not self.__enable:
            return

        if self.__support_lookup_table:
            self.__ibusconn.emit_dbus_signal("CursorUpLookupTable", self.__id)
        else:
            self.emit("cursor-up-lookup-table")

    def __cursor_down_lookup_table_cb(self, engine):
        if not self.__enable:
            return

        if self.__support_lookup_table:
            self.__ibusconn.emit_dbus_signal("CursorDownLookupTable", self.__id)
        else:
            self.emit("cursor-down-lookup-table")

    def __register_properties_cb(self, engine, props):
        if not self.__enable:
            return
        self.emit("register-properties", props)

    def __update_property_cb(self, engine, prop):
        if not self.__enable:
            return
        self.emit("update-property", prop)

    def __remove_engine_handlers(self):
        assert self.__engine != None

        map(self.__engine.disconnect, self.__engine_handlers)
        del self.__engine_handlers[:]

    def __install_engine_handlers(self):
        signals = (
            ("destroy", self.__engine_destroy_cb),
            ("commit-string", self.__commit_string_cb),
            ("update-preedit", self.__update_preedit_cb),
            ("show-preedit", self.__show_preedit_cb),
            ("hide-preedit", self.__hide_preedit_cb),
            ("update-aux-string", self.__update_aux_string_cb),
            ("show-aux-string", self.__show_aux_string_cb),
            ("hide-aux-string", self.__hide_aux_string_cb),
            ("update-lookup-table", self.__update_lookup_table_cb),
            ("show-lookup-table", self.__show_lookup_table_cb),
            ("hide-lookup-table", self.__hide_lookup_table_cb),
            ("page-up-lookup-table", self.__page_up_lookup_table_cb),
            ("page-down-lookup-table", self.__page_down_lookup_table_cb),
            ("cursor-up-lookup-table", self.__cursor_up_lookup_table_cb),
            ("cursor-down-lookup-table", self.__cursor_down_lookup_table_cb),
            ("register-properties", self.__register_properties_cb),
            ("update-property", self.__update_property_cb)
        )

        for signal, handler in signals:
            id = self.__engine.connect(signal, handler)
            self.__engine_handlers.append(id)

gobject.type_register(InputContext)
