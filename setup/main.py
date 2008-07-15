# vim:set et ts=4:
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

from os import path
import gtk
import gobject
import ibus
from gtk import gdk, glade
import dbus.mainloop.glib

(
    NAME_COLUMN,
    ENABLE_COLUMN,
    VISIBLE_COLUMN,
    ICON_COLUMN,
    DATA_COLUMN,
) = range(5)

class Setup(object):
    def __init__(self):
        super(Setup, self).__init__()
        self._conn = ibus.Connection()
        self._ibus = self._conn.get_ibus()
        glade_file = path.join(path.dirname(__file__), "./setup.glade")
        self._xml = glade.XML(glade_file)
        self._dialog = self._xml.get_widget("dialog_setup")
        self._tree = self._xml.get_widget("treeview_engines")
        model = self.__create_model()
        self._tree.set_model(model)

        # add icon search path
        icon_theme = gtk.icon_theme_get_default()
        dir = path.dirname(__file__)
        icondir = path.join(dir, "..", "icons")
        icon_theme.prepend_search_path(icondir)

        # column for holiday names
        column = gtk.TreeViewColumn()
        column.set_title("Name")

        renderer = gtk.CellRendererPixbuf()
        renderer.set_property("xalign", 0.5)

        column.pack_start(renderer)
        column.set_attributes(renderer, icon_name = ICON_COLUMN, visible = VISIBLE_COLUMN)

        renderer = gtk.CellRendererText()
        renderer.set_property("xalign", 0.0)

        # column.set_clickable(True)
        column.pack_start(renderer)
        column.set_attributes(renderer, text = NAME_COLUMN)

        self._tree.append_column(column)

        # column for holiday names
        renderer = gtk.CellRendererToggle()
        renderer.set_data('column', ENABLE_COLUMN)
        renderer.set_property("xalign", 0.5)
        renderer.connect("toggled", self.__item_toggled_cb, model)

        #col_offset = gtk.TreeViewColumn("Holiday", renderer, text=HOLIDAY_NAME_COLUMN)
        column = gtk.TreeViewColumn("Started", renderer, active = ENABLE_COLUMN, visible = VISIBLE_COLUMN)
        column.set_clickable(True)
        column.set_resizable(False)
        column.set_fixed_width(50)
        self._tree.append_column(column)


    def _init_ui(self):
        model = self.__create_model()

    def __item_toggled_cb(self, cell, path_str, model):

        # get toggled iter
        iter = model.get_iter_from_string(path_str)
        data = model.get_value(iter, DATA_COLUMN)

        # do something with the value
        if data[6] == False:
            try:
                self._ibus.RegisterStartEngine(data[1], data[0])
            except Exception, e:
                dlg = gtk.MessageDialog(type = gtk.MESSAGE_ERROR,
                        buttons = gtk.BUTTONS_CLOSE,
                        message_format = str(e))
                dlg.run()
                return
        else:
            try:
                self._ibus.RegisterStopEngine(data[1], data[0])
            except Exception, e:
                dlg = gtk.MessageDialog(type = gtk.MESSAGE_ERROR,
                        buttons = gtk.BUTTONS_CLOSE,
                        message_format = str(e))
                dlg.run()
                return
        data[6] = not data[6]

        # set new value
        model.set(iter, ENABLE_COLUMN, data[6])


    def __create_model(self):
        # create tree store
        model = gtk.TreeStore(
            gobject.TYPE_STRING,
            gobject.TYPE_BOOLEAN,
            gobject.TYPE_BOOLEAN,
            gobject.TYPE_STRING,
            gobject.TYPE_PYOBJECT)

        langs = dict()

        for name, lang, icon, author, credits, _exec, started in self._ibus.RegisterListEngines():
            _lang = ibus.LANGUAGES.get(lang, "other")
            if _lang not in langs:
                langs[_lang] = []
            langs[_lang].append([name, lang, icon, author, credits, _exec, started])

        keys = langs.keys()
        keys.sort()
        for key in keys:
            iter = model.append(None)
            model.set(iter,
                NAME_COLUMN, key,
                ENABLE_COLUMN, False,
                VISIBLE_COLUMN, False,
                ICON_COLUMN, None,
                DATA_COLUMN, None)
            langs[key].sort()
            for name, lang, icon, author, credits, _exec, started in langs[key]:
                child_iter = model.append(iter)
                model.set(child_iter,
                    NAME_COLUMN, name,
                    ENABLE_COLUMN, started,
                    VISIBLE_COLUMN, True,
                    ICON_COLUMN, icon,
                    DATA_COLUMN, 
                    [name, lang, icon, author, credits, _exec, started])

        return model


    def run(self):
        return self._dialog.run()

if __name__ == "__main__":
    dbus.mainloop.glib.DBusGMainLoop(set_as_default = True)
    Setup().run()
