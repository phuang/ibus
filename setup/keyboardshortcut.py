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
    "KryboardShortcutSelection",
);

import gtk

_ = lambda a: a

class KeyboardShortcutSelection(gtk.VBox):
    def __init__(self):
        super(KeyboardShortcutSelection, self).__init__()
        self.__init_ui()

    def __init_ui(self):
        label = gtk.Label(_("Keyboard shortcuts:"))
        label.set_justify(gtk.JUSTIFY_LEFT)
        label.set_alignment(0.0, 0.5)
        self.pack_start(label, False, True, 4)

        # shortcuts view
        viewport =  gtk.Viewport()
        viewport.set_shadow_type(gtk.SHADOW_IN)
        self.__shortcut_view = gtk.TreeView()
        self.__shortcut_view.set_size_request(-1, 100)
        viewport.add(self.__shortcut_view)
        self.pack_start(viewport, True, True, 4)

        # key code
        hbox = gtk.HBox()
        label = gtk.Label(_("Key code:"))
        label.set_justify(gtk.JUSTIFY_LEFT)
        label.set_alignment(0.0, 0.5)
        hbox.pack_start(label, False, True, 4)

        self.__keycode_entry = gtk.Entry()
        hbox.pack_start(self.__keycode_entry, True, True, 4)
        self.__keycode_button = gtk.Button("...")
        hbox.pack_start(self.__keycode_button, False, True, 4)
        self.pack_start(hbox, False, True, 4)

        # modifiers
        hbox = gtk.HBox()
        label = gtk.Label(_("Modifiers:"))
        label.set_justify(gtk.JUSTIFY_LEFT)
        label.set_alignment(0.0, 0.5)
        hbox.pack_start(label, False, True, 4)

        table = gtk.Table(4, 2)
        self.__modifier_buttons = {}
        self.__modifier_buttons["Ctrl"]     = gtk.CheckButton("_Ctrl")
        self.__modifier_buttons["Alt"]      = gtk.CheckButton("A_lt")
        self.__modifier_buttons["Shift"]    = gtk.CheckButton("_Shift")
        self.__modifier_buttons["Release"]  = gtk.CheckButton("_Release")
        self.__modifier_buttons["Meta"]     = gtk.CheckButton("_Meta")
        self.__modifier_buttons["Super"]    = gtk.CheckButton("S_uper")
        self.__modifier_buttons["Hyper"]    = gtk.CheckButton("_Hyper")
        self.__modifier_buttons["Capslock"] = gtk.CheckButton("Caps_lock")

        table.attach(self.__modifier_buttons["Ctrl"], 0, 1, 0, 1)
        table.attach(self.__modifier_buttons["Alt"], 1, 2, 0, 1)
        table.attach(self.__modifier_buttons["Shift"], 2, 3, 0, 1)
        table.attach(self.__modifier_buttons["Release"], 3, 4, 0, 1)
        table.attach(self.__modifier_buttons["Meta"], 0, 1, 1, 2)
        table.attach(self.__modifier_buttons["Super"], 1, 2, 1, 2)
        table.attach(self.__modifier_buttons["Hyper"], 2, 3, 1, 2)
        table.attach(self.__modifier_buttons["Capslock"], 3, 4, 1, 2)
        hbox.pack_start(table, True, True, 4)
        self.pack_start(hbox, False, True, 4)

        # buttons
        hbox = gtk.HBox()
        self.__add_button = gtk.Button(stock = gtk.STOCK_ADD)
        self.__delete_button = gtk.Button(stock = gtk.STOCK_DELETE)
        hbox.pack_start(self.__add_button)
        hbox.pack_start(self.__delete_button)
        self.pack_start(hbox, False, True, 4)


if __name__ == "__main__":
    w = gtk.Window()
    w.set_border_width(10)

    w.add(KeyboardShortcutSelection())
    w.show_all()
    gtk.main()
