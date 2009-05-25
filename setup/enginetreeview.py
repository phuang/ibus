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

import gtk
import glib
import gobject
import pango
import ibus

from icon import load_icon

class EngineTreeView(gtk.TreeView):
    __gsignals__ = {
        'changed' : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ())
    }

    def __init__(self, engines):
        super(EngineTreeView, self).__init__()

        self.__engines = set([])

        self.__changed = False

        self.__model = gtk.ListStore(gobject.TYPE_PYOBJECT)
        self.set_model(self.__model)
        self.__model.connect("row-changed", self.__emit_changed_delay_cb, "row-changed")
        self.__model.connect("row-deleted", self.__emit_changed_delay_cb, "row-deleted")
        self.__model.connect("row-inserted", self.__emit_changed_delay_cb, "row-inserted")
        self.__model.connect("rows-reordered", self.__emit_changed_delay_cb, "rows-reordered")

        self.set_headers_visible(False)

        column = gtk.TreeViewColumn()

        renderer = gtk.CellRendererPixbuf()
        renderer.set_property("xalign", 0)
        column.pack_start(renderer, False)
        column.set_cell_data_func(renderer, self.__icon_cell_data_cb)

        renderer = gtk.CellRendererText()
        renderer.set_property("xalign", 0)
        column.pack_start(renderer, True)
        column.set_cell_data_func(renderer, self.__name_cell_data_cb)
        self.append_column (column)

        self.set_reorderable(True)

        self.set_engines(engines)

    def __emit_changed(self, *args):
        if self.__changed:
            self.__changed = False
            self.emit("changed")

    def __emit_changed_delay_cb(self, *args):
        if not self.__changed:
            self.__changed = True
            glib.idle_add(self.__emit_changed)


    def __icon_cell_data_cb(self, celllayout, renderer, model, iter):
        engine = self.__model.get_value(iter, 0)

        icon_size = gtk.icon_size_lookup(gtk.ICON_SIZE_LARGE_TOOLBAR)[0]
        pixbuf = load_icon(engine.icon, gtk.ICON_SIZE_LARGE_TOOLBAR)

        if pixbuf == None:
            pixbuf = load_icon("engine-default", gtk.ICON_SIZE_LARGE_TOOLBAR)
        if pixbuf == None:
            pixbuf = load_icon("gtk-missing-image", gtk.ICON_SIZE_LARGE_TOOLBAR)

        renderer.set_property("pixbuf", pixbuf)

    def __name_cell_data_cb(self, celllayout, renderer, model, iter):
        engine = self.__model.get_value(iter, 0)
        renderer.set_property("sensitive", True)
        language = ibus.get_language_name(engine.language)
        renderer.set_property("text", "%s - %s" % (language, engine.longname))
        if self.__model.get_path(iter)[0] == 0:
            #default engine
            renderer.set_property("weight", pango.WEIGHT_BOLD)
        else:
            renderer.set_property("weight", pango.WEIGHT_NORMAL)


    def set_engines(self, engines):
        self.__model.clear()
        self.__engines = set([])
        for e in engines:
            if e in self.__engines:
                continue
            iter = self.__model.append(None)
            self.__model.set(iter, 0, e)
            self.__engines.add(e)
        self.__emit_changed()

    def get_selected_iter(self):
        selection = self.get_selection()
        if selection:
            return selection.get_selected()[1]

    def get_engines(self):
        engines = [ r[0] for r in self.__model if r[0] != None]
        return engines

    def get_select_engine(self):
        iter = self.get_selected_iter()
        if iter == None:
            return None
        row = self.__model.get(iter)
        return row[0]

    def prepend_engine(self, engine):
        if engine == None or engine in self.__engines:
            return
        iter = self.__model.prepend(None)
        self.__model.set(iter, 0, engine)
        self.__engines.add(engine)
        self.scroll_to_cell(self.__model[0].path, None)

    def append_engine(self, engine):
        if engine == None or engine in self.__engines:
            return
        iter = self.__model.append(None)
        self.__model.set(iter, 0, engine)
        self.__engines.add(engine)
        self.scroll_to_cell(self.__model[-1].path, None)

    def remove_engine(self):
        iter = self.get_selected_iter()
        if iter == None:
            return
        row = self.__model[iter]
        engine = row[0]
        self.__engines.remove(engine)
        index = row.path[0]
        self.__model.remove(iter)
        try:
            row = self.__model[index]
            selection = self.get_selection()
            selection.select_path(row.path)
        except:
            pass

    def move_up_engine(self):
        iter = self.get_selected_iter()
        if iter == None:
            return
        row = self.__model[iter]
        index = row.path[0]
        if index == 0:
            return
        self.__model.swap(iter, self.__model[index - 1].iter)
        self.scroll_to_cell(row.path, None)

    def move_down_engine(self):
        iter = self.get_selected_iter()
        if iter == None:
            return
        row = self.__model[iter]
        index = row.path[0]
        last_row = self.__model[-1]
        last_index = last_row.path[0]
        if index == last_index:
            return
        self.__model.swap(iter, self.__model[index + 1].iter)
        self.scroll_to_cell(row.path, None)

gobject.type_register(EngineTreeView)
