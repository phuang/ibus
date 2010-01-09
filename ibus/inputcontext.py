# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2007-2010 Red Hat, Inc.
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
        "InputContext",
    )

import sys
import gobject
import dbus
import dbus.lowlevel
import object
import common
import serializable

class InputContext(object.Object):
    __gtype_name__ = "PYIBusInputContext"
    __gsignals__ = {
        "commit-text" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            (gobject.TYPE_PYOBJECT, )
        ),
        "update-preedit-text" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            (gobject.TYPE_PYOBJECT, gobject.TYPE_UINT, gobject.TYPE_BOOLEAN)
        ),
        "show-preedit-text" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
        "hide-preedit-text" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
        "update-auxiliary-text" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            (gobject.TYPE_PYOBJECT, gobject.TYPE_BOOLEAN)
        ),
        "show-auxiliary-text" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
        "hide-auxiliary-text" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
        "update-lookup-table" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            (gobject.TYPE_PYOBJECT, gobject.TYPE_BOOLEAN)
        ),
        "show-lookup-table" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
        "hide-lookup-table" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
        "page-up-lookup-table" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
        "page-down-lookup-table" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
        "cursor-up-lookup-table" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
        "cursor-down-lookup-table" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
        "enabled" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
        "disabled" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
    }

    def __init__(self, bus, path, watch_signals=False):
        super(InputContext, self).__init__()

        self.__bus = bus
        self.__context = bus.get_dbusconn().get_object(common.IBUS_SERVICE_IBUS, path)
        self.__signal_matches = []

        if not watch_signals:
            return

        m = self.__context.connect_to_signal("CommitText", self.__commit_text_cb)
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("UpdatePreeditText", self.__update_preedit_text_cb)
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("UpdateAuxiliaryText", self.__update_auxiliary_text_cb)
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("UpdateLookupTable", self.__update_lookup_table_cb)
        self.__signal_matches.append(m)

        m = self.__context.connect_to_signal("Enabled",             lambda *args: self.emit("enabled"))
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("Disabled",            lambda *args: self.emit("disabled"))
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("ShowPreeditText",     lambda *args: self.emit("show-preedit-text"))
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("HidePreeditText",     lambda *args: self.emit("hide-preedit-text"))
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("ShowAuxiliaryText",   lambda *args: self.emit("show-auxiliary-text"))
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("HideAuxiliaryText",   lambda *args: self.emit("hide-auxiliary-text"))
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("ShowLookupTable",     lambda *args: self.emit("show-lookup-table"))
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("HideLookupTable",     lambda *argss: self.emit("hide-lookup-table"))
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("PageUpLookupTable",   lambda *args: self.emit("page-up-lookup-table"))
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("PageDownLookupTable", lambda *args: self.emit("page-down-lookup-table"))
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("CursorUpLookupTable", lambda *args: self.emit("cursor-up-lookup-table"))
        self.__signal_matches.append(m)
        m = self.__context.connect_to_signal("CursorDownLookupTable", lambda *args: self.emit("cursor-down-lookup-table"))
        self.__signal_matches.append(m)

    def __commit_text_cb(self, *args):
        text = serializable.deserialize_object(args[0])
        self.emit("commit-text", text)

    def __update_preedit_text_cb(self, *args):
        text = serializable.deserialize_object(args[0])
        cursor_pos = args[1]
        visible = args[2]
        self.emit("update-preedit-text", text, cursor_pos, visible)

    def __update_auxiliary_text_cb(self, *args):
        text = serializable.deserialize_object(args[0])
        visible = args[1]
        self.emit("update-auxiliary-text", text, visible)

    def __update_lookup_table_cb(self, *args):
        table = serializable.deserialize_object(args[0])
        visible = args[1]
        self.emit("update-lookup-table", table, visible)

    def process_key_event(self, keyval, keycode, modifiers):
        keyval = dbus.UInt32(keyval)
        keycode = dbus.UInt32(keycode)
        modifiers = dbus.UInt32(modifiers)
        return self.__context.ProcessKeyEvent(keyval, keycode, modifiers)

    def set_cursor_location(self, x, y, w, h):
        x = dbus.Int32(x)
        y = dbus.Int32(y)
        w = dbus.Int32(w)
        h = dbus.Int32(h)
        return self.__context.SetCursorLocation(x, y, w, h)

    def focus_in(self):
        return self.__context.FocusIn()

    def focus_out(self):
        return self.__context.FocusOut()

    def reset(self):
        return self.__context.Reset()

    def enable(self):
        return self.__context.Enable()

    def disable(self):
        return self.__context.Disable()

    def is_enabled(self):
        return self.__context.IsEnabled()

    def set_capabilities(self, caps):
        caps = dbus.UInt32(caps)
        return self.__context.SetCapabilities(caps)

    def detach_signals(self):
        for m in self.__signal_matches:
            m.remove()
        del self.__signal_matches[:]

    def destroy(self):
        self.detach_signals()
        self.__context.Destroy()
        super(InputContext, self).destroy()

    def get_engine(self):
        try:
            engine = self.__context.GetEngine()
            engine = serializable.deserialize_object(engine)
            return engine
        except:
            return None

    def set_engine(self, engine):
        return self.__context.SetEngine(engine.name)

    def introspect(self):
        return self.__context.Introspect()



def test():
    import gtk
    import gtk.gdk
    from bus import Bus
    import modifier
    import text
    import attribute
    import property
    import lookuptable
    import factory

    class TestWindow(gtk.Window):
        def __init__(self):
            super(TestWindow,self).__init__()

            self.__bus = Bus()
            print self.__bus.get_name()
            self.__bus.connect("disconnected", gtk.main_quit)
            context_path = self.__bus.create_input_context("Test")
            print context_path
            self.__context = InputContext(self.__bus, context_path)
            self.__context.set_capabilities (9)

            self.__context.connect("commit-text", self.__commit_text_cb)
            self.__context.connect("update-preedit-text", self.__update_preedit_text_cb)
            self.__context.connect("show-preedit-text", self.__show_preedit_text_cb)
            self.__context.connect("update-auxiliary-text", self.__update_auxiliary_text_cb)
            self.__context.connect("update-lookup-table", self.__update_lookup_table_cb)
            self.__context.connect("enabled", self.__enabled_cb)
            self.__context.connect("disabled", self.__disabled_cb)

            self.set_events(gtk.gdk.KEY_PRESS_MASK | gtk.gdk.KEY_RELEASE_MASK | gtk.gdk.FOCUS_CHANGE_MASK)

            self.connect("key-press-event", self.__key_press_event_cb)
            self.connect("key-release-event", self.__key_release_event_cb)
            self.connect("delete-event", gtk.main_quit)
            self.connect("focus-in-event", lambda *args: self.__context.focus_in())
            self.connect("focus-out-event", lambda *args: self.__context.focus_out())

            self.show_all()

        def __commit_text_cb(self, context, text):
            print "commit-text:", text.text

        def __update_preedit_text_cb(self, context, text, cursor_pos, visible):
            print "preedit-text:", text.text, cursor_pos, visible

        def __show_preedit_text_cb(self, context):
            print "show-preedit-text"

        def __hide_preedit_text_cb(self, context):
            print "hide-preedit-text"

        def __update_auxiliary_text_cb(self, context, text, visible):
            print "auxiliary-text:", text.text, visible

        def __update_lookup_table_cb(self, context, table, visible):
            print "update-lookup-table:", visible

        def __enabled_cb(self, context):
            print "enabled"
            info = context.get_factory_info()
            print "factory = %s" % info.name

        def __disabled_cb(self, context):
            print "disabled"

        def __key_press_event_cb(self, widget, event):
            self.__context.process_key_event(event.keyval, event.state)

        def __key_release_event_cb(self, widget, event):
            self.__context.process_key_event(event.keyval, event.state | modifier.RELEASE_MASK)

    w = TestWindow()
    gtk.main()

if __name__ == "__main__":
    test()

