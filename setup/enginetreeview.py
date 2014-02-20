# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2014 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2007-2014 Red Hat, Inc.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
# USA

from gi.repository import GLib
from gi.repository import GObject
from gi.repository import Gtk
from gi.repository import IBus
from gi.repository import Pango

from icon import load_icon
from i18n import _, N_

class EngineTreeView(Gtk.TreeView):
    __gtype_name__ = 'EngineTreeView'
    __gproperties__ = {
        'active-engine' : (
            object,
            'selected engine',
            'selected engine',
            GObject.ParamFlags.READABLE),
        'engines' : (
            object,
            'engines',
            'engines',
            GObject.ParamFlags.READABLE | GObject.ParamFlags.WRITABLE)
    }

    def __init__(self):
        super(EngineTreeView, self).__init__()

        self.__engines = set([])
        self.__changed = False

        # self.set_headers_visible(True)
        self.set_reorderable(True)

        self.__model = Gtk.ListStore(GObject.TYPE_PYOBJECT, GObject.TYPE_STRING)
        self.set_model(self.__model)
        self.__model.connect("row-changed", self.__emit_changed_delay_cb, "row-changed")
        self.__model.connect("row-deleted", self.__emit_changed_delay_cb, "row-deleted")
        self.__model.connect("row-inserted", self.__emit_changed_delay_cb, "row-inserted")
        self.__model.connect("rows-reordered", self.__emit_changed_delay_cb, "rows-reordered")
        self.__model.set_default_sort_func(self.__sort_engines, None)
        self.__model.set_sort_column_id(-1, Gtk.SortType.ASCENDING)

        # create im name & icon column
        column = Gtk.TreeViewColumn(_("Input Method"))
        column.set_min_width(220)

        renderer = Gtk.CellRendererPixbuf()
        renderer.set_property("xalign", 0)
        column.pack_start(renderer, False)
        column.set_cell_data_func(renderer, self.__icon_cell_data_cb)

        renderer = Gtk.CellRendererText()
        renderer.set_property("xalign", 0)
        renderer.set_property("ellipsize", Pango.EllipsizeMode.END)
        column.pack_start(renderer, True)
        column.set_cell_data_func(renderer, self.__name_cell_data_cb)
        self.append_column(column)

        # create im keyboard layout column
        renderer = Gtk.CellRendererCombo()
        model = Gtk.ListStore(GObject.TYPE_STRING)
        model.append(("us",))
        model.append(("jp",))
        model.append(("xkb",))
        renderer.set_property("xalign", 0)
        renderer.set_property("model", model)
        renderer.set_property("text-column", 0)
        renderer.set_property("has-entry", False)
        renderer.set_property("editable", True)
        renderer.connect("changed", self.__engine_layout_changed_cb)

        column = Gtk.TreeViewColumn(_("Kbd"))
        column.set_expand(False)
        column.set_fixed_width(32)
        column.set_sizing(Gtk.TreeViewColumnSizing.FIXED)
        column.pack_start(renderer, False)
        column.set_cell_data_func(renderer, self.__layout_cell_data_cb)
        # self.append_column(column)

        self.get_selection().connect("changed", self.__selection_changed_cb)

    def __sort_engines(self, model, a, b, data):
        engine_a = model[a][0]
        engine_b = model[b][0]
        language_a = IBus.get_language_name(engine_a.get_language())
        language_b = IBus.get_language_name(engine_b.get_language())
        label_a = "%s - %s" % (language_a, engine_a.get_longname())
        label_b = "%s - %s" % (language_b, engine_b.get_longname())
        # http://docs.python.org/3.0/whatsnew/3.0.html#ordering-comparisons
        return (label_a > label_b) - (label_a < label_b)

    def __selection_changed_cb(self, *args):
        self.notify("active-engine");

    def __emit_changed(self, *args):
        if self.__changed:
            self.__changed = False
            self.notify("engines")

    def __emit_changed_delay_cb(self, *args):
        if not self.__changed:
            self.__changed = True
            GLib.idle_add(self.__emit_changed)


    def __icon_cell_data_cb(self, celllayout, renderer, model, it, data):
        engine = self.__model.get_value(it, 0)

        icon_size = Gtk.icon_size_lookup(Gtk.IconSize.LARGE_TOOLBAR)[0]
        pixbuf = load_icon(engine.get_icon(), Gtk.IconSize.LARGE_TOOLBAR)
        renderer.set_property("pixbuf", pixbuf)

    def __name_cell_data_cb(self, celllayout, renderer, model, it, data):
        engine = self.__model.get_value(it, 0)
        renderer.set_property("sensitive", True)
        language = IBus.get_language_name(engine.get_language())
        renderer.set_property("text",
                "%s - %s" % (language, engine.get_longname()))
        if self.__model.get_path(it).get_indices()[0] == 0:
            # default engine
            renderer.set_property("weight", Pango.Weight.BOLD)
        else:
            renderer.set_property("weight", Pango.Weight.NORMAL)

    def __layout_cell_data_cb(self, celllayout, renderer, model, it, data):
        engine = self.__model.get_value(it, 0)
        layout = self.__model.get_value(it, 1)
        renderer.set_property("sensitive", True)
        if not layout:
            layout = engine.layout
        renderer.set_property("text", layout)
        if self.__model.get_path(it).get_indices()[0] == 0:
            #default engine
            renderer.set_property("weight", Pango.Weight.BOLD)
        else:
            renderer.set_property("weight", Pango.Weight.NORMAL)

    def __engine_layout_changed_cb(self, combo, path, it):
        return
        i = self.__model.get_iter(path)
        layout = combo.get_property("model").get_value(it, 0)
        self.__model.set_value(i, 1, layout)

    def do_get_property(self, prop):
        if prop.name == "active-engine":
            it = self.get_selected_iter()
            if it == None:
                return None
            row = self.__model.get(it, 0)
            return row[0]
        elif prop.name == "engines":
            engines = [ r[0] for r in self.__model if r[0] != None]
            return engines
        else:
            raise AttributeError('unknown property %s' % prop.name)

    def do_set_property(self, prop, value):
        if prop.name == "active-engine":
            raise AttributeError("active-engine is readonly")
        elif prop.name == "engines":
            set_engines(value)
        else:
            raise AttributeError('unknown property %s' % prop.name)

    def set_engines(self, engines):
        self.__model.clear()
        self.__engines = set([])
        for e in engines:
            if e in self.__engines:
                continue
            it = self.__model.append(None)
            self.__model.set(it, 0, e)
            self.__engines.add(e)
        self.__emit_changed()

    def get_selected_iter(self):
        selection = self.get_selection()
        if selection:
            return selection.get_selected()[1]

    def get_engines(self):
        return self.get_property("engines")

    def get_active_engine(self):
        return self.get_property("active-engine")

    def prepend_engine(self, engine):
        if engine == None or engine in self.__engines:
            return
        it = self.__model.prepend(None)
        self.__model.set(it, 0, engine)
        self.__engines.add(engine)
        self.scroll_to_cell(self.__model[0].path, None)

    def append_engine(self, engine):
        if engine == None or engine in self.__engines:
            return
        it = self.__model.append(None)
        self.__model.set(it, 0, engine)
        self.__engines.add(engine)
        self.scroll_to_cell(self.__model[-1].path, None)

    def remove_engine(self):
        it = self.get_selected_iter()
        if it == None:
            return
        row = self.__model[it]
        engine = row[0]
        self.__engines.remove(engine)
        index = row.path.get_indices()[0]
        self.__model.remove(it)
        try:
            row = self.__model[index]
            selection = self.get_selection()
            selection.select_path(row.path)
        except:
            pass

    def move_up_engine(self):
        it = self.get_selected_iter()
        if it == None:
            return
        row = self.__model[it]
        index = row.path.get_indices()[0]
        if index == 0:
            return
        self.__model.swap(it, self.__model[index - 1].iter)
        self.scroll_to_cell(row.path, None)

    def move_down_engine(self):
        it = self.get_selected_iter()
        if it == None:
            return
        row = self.__model[it]
        index = row.path.get_indices()[0]
        last_row = self.__model[-1]
        last_index = last_row.path.get_indices()[0]
        if index == last_index:
            return
        self.__model.swap(it, self.__model[index + 1].iter)
        self.scroll_to_cell(row.path, None)

GObject.type_register(EngineTreeView)

if __name__ == "__main__":
    tree = EngineTreeView()
    tree.set_engines([IBus.EngineDesc(language="zh")])
    w = Gtk.Window()
    w.add(tree)
    w.show_all()
    Gtk.main()
