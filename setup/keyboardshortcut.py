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
    "KeyboardShortcutSelection",
    "KeyboardShortcutSelectionDialog",
);

from gi.repository import Gdk
from gi.repository import GObject
from gi.repository import Gtk
from gi.repository import IBus
from gi.repository import Pango

from i18n import _, N_

MAX_HOTKEY = 6

class KeyboardShortcutSelection(Gtk.VBox):
    def __init__(self, shortcuts = None):
        super(KeyboardShortcutSelection, self).__init__()
        self.__init_ui()
        self.set_shortcuts(shortcuts)

    def __init_ui(self):
        # label = Gtk.Label(_("Keyboard shortcuts:"))
        # label.set_justify(Gtk.Justification.LEFT)
        # label.set_alignment(0.0, 0.5)
        # self.pack_start(label, False, True, 4)

        # shortcuts view
        self.__shortcut_view = Gtk.TreeView(Gtk.ListStore(GObject.TYPE_STRING))
        self.__shortcut_view.set_size_request(-1, 100)
        renderer = Gtk.CellRendererText()
        column = Gtk.TreeViewColumn(_("Keyboard shortcuts"), renderer, text = 0)
        self.__shortcut_view.append_column(column)
        self.__shortcut_view.connect("cursor-changed", self.__shortcut_view_cursor_changed_cb)
        scrolledwindow = Gtk.ScrolledWindow()
        scrolledwindow.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        scrolledwindow.add(self.__shortcut_view)
        scrolledwindow.set_shadow_type(Gtk.ShadowType.IN)
        self.pack_start(scrolledwindow, True, True, 4)

        # key code
        hbox = Gtk.HBox()
        label = Gtk.Label(_("Key code:"))
        label.set_justify(Gtk.Justification.LEFT)
        label.set_alignment(0.0, 0.5)
        hbox.pack_start(label, False, True, 4)

        self.__keycode_entry = Gtk.Entry()
        self.__keycode_entry.connect("notify::text", self.__keycode_entry_notify_cb)
        hbox.pack_start(self.__keycode_entry, True, True, 4)
        self.__keycode_button = Gtk.Button("...")
        self.__keycode_button.connect("clicked", self.__keycode_button_clicked_cb)
        hbox.pack_start(self.__keycode_button, False, True, 4)
        self.pack_start(hbox, False, True, 4)

        # modifiers
        hbox = Gtk.HBox()
        label = Gtk.Label(_("Modifiers:"))
        label.set_justify(Gtk.Justification.LEFT)
        label.set_alignment(0.0, 0.5)
        hbox.pack_start(label, False, True, 4)

        table = Gtk.Table(4, 2)
        self.__modifier_buttons = []
        self.__modifier_buttons.append(("Control",  Gtk.CheckButton("_Control"),    Gdk.ModifierType.CONTROL_MASK))
        self.__modifier_buttons.append(("Alt",      Gtk.CheckButton("A_lt"),        Gdk.ModifierType.MOD1_MASK))
        self.__modifier_buttons.append(("Shift",    Gtk.CheckButton("_Shift"),      Gdk.ModifierType.SHIFT_MASK))
        self.__modifier_buttons.append(("Meta",     Gtk.CheckButton("_Meta"),       Gdk.ModifierType.META_MASK))
        self.__modifier_buttons.append(("Super",    Gtk.CheckButton("S_uper"),      Gdk.ModifierType.SUPER_MASK))
        self.__modifier_buttons.append(("Hyper",    Gtk.CheckButton("_Hyper"),      Gdk.ModifierType.HYPER_MASK))
        self.__modifier_buttons.append(("Capslock", Gtk.CheckButton("Capsloc_k"),   Gdk.ModifierType.LOCK_MASK))
        self.__modifier_buttons.append(("Release",  Gtk.CheckButton("_Release"),    Gdk.ModifierType.RELEASE_MASK))
        for name, button, mask in self.__modifier_buttons:
            button.connect("toggled", self.__modifier_button_toggled_cb, name)

        table.attach(self.__modifier_buttons[0][1], 0, 1, 0, 1)
        table.attach(self.__modifier_buttons[1][1], 1, 2, 0, 1)
        table.attach(self.__modifier_buttons[2][1], 2, 3, 0, 1)
        table.attach(self.__modifier_buttons[3][1], 3, 4, 0, 1)
        table.attach(self.__modifier_buttons[4][1], 0, 1, 1, 2)
        table.attach(self.__modifier_buttons[5][1], 1, 2, 1, 2)
        table.attach(self.__modifier_buttons[6][1], 2, 3, 1, 2)
        table.attach(self.__modifier_buttons[7][1], 3, 4, 1, 2)
        hbox.pack_start(table, True, True, 4)
        self.pack_start(hbox, False, True, 4)

        # buttons
        hbox = Gtk.HBox()
        # add button
        self.__add_button = Gtk.Button(stock = Gtk.STOCK_ADD)
        self.__add_button.set_sensitive(False)
        self.__add_button.connect("clicked", self.__add_button_clicked_cb)
        hbox.pack_start(self.__add_button, False, True, 0)
        # apply button
        self.__apply_button = Gtk.Button(stock = Gtk.STOCK_APPLY)
        self.__apply_button.set_sensitive(False)
        self.__apply_button.connect("clicked", self.__apply_button_clicked_cb)
        hbox.pack_start(self.__apply_button, False, True, 0)
        # delete button
        self.__delete_button = Gtk.Button(stock = Gtk.STOCK_DELETE)
        self.__delete_button.set_sensitive(False)
        self.__delete_button.connect("clicked", self.__delete_button_clicked_cb)
        hbox.pack_start(self.__delete_button, False, True, 0)
        self.pack_start(hbox, False, True, 4)

    def set_shortcuts(self, shortcuts = None):
        if shortcuts == None:
            shortcuts = []
        model = self.__shortcut_view.get_model()
        model.clear()

        added = []
        for shortcut in shortcuts:
            if shortcut not in added:
                model.insert(-1, (shortcut,))
                added.append(shortcut)

    def get_shortcuts(self):
        model = self.__shortcut_view.get_model()
        try:
            return [i[0] for i in model]
        except:
            return []

    def add_shortcut(self, shortcut):
        model = self.__shortcut_view.get_model()
        if len(model) > MAX_HOTKEY:
            return
        if shortcut in self.get_shortcuts():
            return
        iter = model.insert(0)
        model[iter][0] = shortcut
        self.__add_button.set_sensitive(False)
        path = model.get_path(iter)
        self.__shortcut_view.set_cursor(path, None, False)

    def __get_shortcut_from_buttons(self):
        modifiers = []
        keycode = self.__keycode_entry.get_text()
        if Gdk.keyval_from_name(keycode) == 0:
            return None

        for name, button, mask in self.__modifier_buttons:
            if button.get_active():
                modifiers.append(name)
        if keycode.startswith("_"):
            keycode = keycode[1:]
        keys = modifiers + [keycode]
        shortcut = "+".join(keys)
        return shortcut

    def __set_shortcut_to_buttons(self, shortcut):
        keys = shortcut.split("+")
        mods = keys[:-1]
        for name, button, mask in self.__modifier_buttons:
            if name in mods:
                button.set_active(True)
            else:
                button.set_active(False)
        self.__keycode_entry.set_text(keys[-1])

    def __get_selected_shortcut(self):
        model = self.__shortcut_view.get_model()
        path, column = self.__shortcut_view.get_cursor()
        if path == None:
            return None
        return model[path.get_indices()[0]][0]

    def __set_selected_shortcut(self, shortcut):
        model = self.__shortcut_view.get_model()
        path, column = self.__shortcut_view.get_cursor()
        model[path[0]][0] = shortcut
        self.__update_add_and_apply_buttons()

    def __del_selected_shortcut(self):
        model = self.__shortcut_view.get_model()
        path, column = self.__shortcut_view.get_cursor()
        del model[path[0]]
        self.__update_add_and_apply_buttons()

    def __shortcut_view_cursor_changed_cb(self, treeview):
        shortcut = self.__get_selected_shortcut()
        self.__set_shortcut_to_buttons(shortcut)
        if shortcut != None:
            self.__delete_button.set_sensitive(True)
        else:
            self.__delete_button.set_sensitive(False)

    def __update_add_and_apply_buttons(self):
        shortcut = self.__get_shortcut_from_buttons()
        selected_shortcut = self.__get_selected_shortcut()
        shortcuts = self.get_shortcuts()
        can_add = shortcut != None and \
                  shortcut not in shortcuts \
                  and len(shortcuts) < MAX_HOTKEY
        self.__add_button.set_sensitive(can_add)
        can_apply = shortcut != selected_shortcut and \
                    shortcut != None and \
                    selected_shortcut != None and \
                    shortcut not in shortcuts
        self.__apply_button.set_sensitive(can_apply)

    def __modifier_button_toggled_cb(self, button, name):
        self.__update_add_and_apply_buttons()

    def __keycode_entry_notify_cb(self, entry, arg):
        self.__update_add_and_apply_buttons()

    def __keycode_button_clicked_cb(self, button):
        out = []
        dlg = Gtk.MessageDialog(parent = self.get_toplevel(), buttons = Gtk.BUTTONS_CLOSE)
        message = _("Please press a key (or a key combination).\nThe dialog will be closed when the key is released.")
        dlg.set_markup(message)
        dlg.set_title(_("Please press a key (or a key combination)"))

        def __key_press_event(d, k, out):
            out.append(k.copy())

        def __key_release_event(d, k, out):
            d.response(Gtk.RESPONSE_OK)

        dlg.connect("key-press-event", __key_press_event, out)
        dlg.connect("key-release-event", __key_release_event, None)
        id = dlg.run()
        dlg.destroy()
        if id != Gtk.RESPONSE_OK or not out:
            return
        keyevent = out[len(out) - 1]
        state = keyevent.state & (Gdk.CONTROL_MASK | \
                                  Gdk.SHIFT_MASK   | \
                                  Gdk.MOD1_MASK    | \
                                  Gdk.META_MASK    | \
                                  Gdk.SUPER_MASK   | \
                                  Gdk.HYPER_MASK)
        if state == 0:
            state = state | Gdk.RELEASE_MASK
        elif keyevent.keyval in (keysyms.Control_L, keysyms.Control_R) and state == Gdk.CONTROL_MASK:
            state = state | Gdk.RELEASE_MASK
        elif keyevent.keyval in (keysyms.Shift_L, keysyms.Shift_R) and state == Gdk.SHIFT_MASK:
            state = state | Gdk.RELEASE_MASK
        elif keyevent.keyval in (keysyms.Alt_L, keysyms.Alt_R) and state == Gdk.MOD1_MASK:
            state = state | Gdk.RELEASE_MASK
        elif keyevent.keyval in (keysyms.Meta_L, keysyms.Meta_R) and state == Gdk.META_MASK:
            state = state | Gdk.RELEASE_MASK
        elif keyevent.keyval in (keysyms.Super_L, keysyms.Super_R) and state == Gdk.SUPER_MASK:
            state = state | Gdk.RELEASE_MASK
        elif keyevent.keyval in (keysyms.Hyper_L, keysyms.Hyper_R) and state == Gdk.HYPER_MASK:
            state = state | Gdk.RELEASE_MASK

        for name, button, mask in self.__modifier_buttons:
            if state & mask:
                button.set_active(True)
            else:
                button.set_active(False)
        self.__keycode_entry.set_text(Gdk.keyval_name(keyevent.keyval))

    def __add_button_clicked_cb(self, button):
        shortcut = self.__get_shortcut_from_buttons()
        self.add_shortcut(shortcut)

    def __apply_button_clicked_cb(self, button):
        shortcut = self.__get_shortcut_from_buttons()
        self.__set_selected_shortcut(shortcut)

    def __delete_button_clicked_cb(self, button):
        self.__del_selected_shortcut()
        self.__delete_button.set_sensitive(False)
        self.__apply_button.set_sensitive(False)

class KeyboardShortcutSelectionDialog(Gtk.Dialog):
    def __init__(self, title = None, parent = None, flags = 0, buttons = None):
        super(KeyboardShortcutSelectionDialog, self).__init__(title, parent, flags, buttons)
        self.__selection_view = KeyboardShortcutSelection()
        self.vbox.pack_start(self.__selection_view, False, True, 0)
        self.vbox.show_all()

    def set_shortcuts(self, shotrcuts = None):
        self.__selection_view.set_shortcuts(shotrcuts)

    def add_shortcut(self, shotrcut):
        self.__selection_view.add_shortcut(shotrcut)

    def get_shortcuts(self):
        return self.__selection_view.get_shortcuts()



if __name__ == "__main__":
    dlg = KeyboardShortcutSelectionDialog(
        title = "Select test",
        buttons = (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                   Gtk.STOCK_OK, Gtk.ResponseType.OK))
    dlg.add_shortcut("Control+Shift+space")
    dlg.set_shortcuts(None)
    print dlg.run()
    print dlg.get_shortcuts()

