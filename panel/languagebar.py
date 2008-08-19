# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright(c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or(at your option) any later version.
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

import gtk
import gtk.gdk as gdk
import gobject
import ibus
from handle import Handle
from menu import menu_position
from toolitem import ToolButton,\
    ToggleToolButton, \
    SeparatorToolItem, \
    MenuToolButton

ICON_SIZE = gtk.ICON_SIZE_MENU

class LanguageBar(gtk.Toolbar):
    __gsignals__ = {
        "property-activate" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_INT)),
        "get-im-menu" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_PYOBJECT,
            ()),
        }

    def __init__ (self):
        super(LanguageBar, self).__init__()
        self.set_style(gtk.TOOLBAR_ICONS)
        self.set_show_arrow(False)
        self.set_property("icon-size", ICON_SIZE)
        self.__create_ui()

        self.__properties = []
        self.__toplevel = gtk.Window(gtk.WINDOW_POPUP)
        self.__toplevel.add(self)

        root = gdk.get_default_root_window()
        try:
            workarea = root.property_get("_NET_WORKAREA")[2]
            right, bottom = workarea[2], workarea[3]
        except:
            right, bottom = 640, 480

        self.__toplevel.move(right - 200, bottom - 40)

    def __create_ui(self):
        # create move handle
        self.__handle = gtk.ToolItem()
        self.__handle.add(Handle())
        self.insert(self.__handle, -1)

        # create input methods menu
        self.__im_menu = ToggleToolButton(ibus.Property(name = "", type = ibus.PROP_TYPE_TOGGLE, icon = "ibus", tooltip = "Swicth engine"))
        self.__im_menu.set_homogeneous(False)
        self.__im_menu.connect("toggled", self.__im_menu_toggled_cb)
        self.insert(self.__im_menu, -1)

    def __im_menu_toggled_cb(self, widget):
        if self.__im_menu.get_active():
            menu = self.emit("get-im-menu")
            menu.connect("deactivate", self.__im_menu_deactivate_cb)
            menu.popup(None, None,
                menu_position,
                0,
                gtk.get_current_event_time(),
                widget)
    def __im_menu_deactivate_cb(self, menu):
        self.__im_menu.set_active(False)

    def __remove_properties(self):
        # reset all properties

        map(lambda i: i.destroy(), self.__properties)
        self.__properties = []

    def do_show(self):
        gtk.Toolbar.do_show(self)

    def do_size_request(self, requisition):
        gtk.Toolbar.do_size_request(self, requisition)
        self.__toplevel.resize(1, 1)

    def set_im_icon(self, icon_name):
        self.__im_menu.set_icon_name(icon_name)

    def reset(self):
        self.__remove_properties()

    def register_properties(self, props):
        self.__remove_properties()
        # create new properties
        for prop in props:
            if prop.type == ibus.PROP_TYPE_NORMAL:
                item = ToolButton(prop = prop)
            elif prop.type == ibus.PROP_TYPE_TOGGLE:
                item = ToggleToolButton(prop = prop)
            elif prop.type == ibus.PROP_TYPE_MENU:
                item = MenuToolButton(prop = prop)
            elif prop.type == PROP_TYPE_SEPARATOR:
                item = SeparatorToolItem()
            else:
                raise IBusException("Unknown property type = %d" % prop.type)

            item.connect("property-activate",
                        lambda w, n, s: self.emit("property-activate", n, s))

            item.set_sensitive(prop.sensitive)

            item.set_no_show_all(True)

            if prop.visible:
                item.show()
            else:
                item.hide()

            self.__properties.append(item)
            self.insert(item, -1)

    def update_property(self, prop):
        map(lambda x: x.update_property(prop), self.__properties)

    def show_all(self):
        self.__toplevel.show_all()
        gtk.Toolbar.show_all(self)

    def hide_all(self):
        self.__toplevel.hide_all()
        gtk.Toolbar.hide_all(self)

    def focus_in(self):
        self.__im_menu.set_sensitive(True)
        self.__toplevel.window.raise_()

    def focus_out(self):
        self.__im_menu.set_sensitive(False)

gobject.type_register(LanguageBar, "IBusLanguageBar")

