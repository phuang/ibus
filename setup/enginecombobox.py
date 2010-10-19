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

import gtk
import gobject
import pango
import ibus
import locale
from icon import load_icon
from i18n import _, N_

class EngineComboBox(gtk.ComboBox):
    __gtype_name__ = 'EngineComboBox'
    __gproperties__ = {
        'active-engine' : (
            gobject.TYPE_PYOBJECT,
            'selected engine',
            'selected engine',
            gobject.PARAM_READABLE)
    }

    def __init__(self):
        super(EngineComboBox, self).__init__()
        self.connect("notify::active", self.__notify_active_cb)

        self.__model = None

        renderer = gtk.CellRendererPixbuf()
        renderer.set_property("xalign", 0)
        renderer.set_property("xpad", 2)
        self.pack_start(renderer, False)
        self.set_cell_data_func(renderer, self.__icon_cell_data_cb)

        renderer = gtk.CellRendererText()
        renderer.set_property("xalign", 0)
        renderer.set_property("xpad", 2)
        self.pack_start(renderer, True)
        self.set_cell_data_func(renderer, self.__name_cell_data_cb)

    def set_engines(self, engines):
        self.__model = gtk.TreeStore(gobject.TYPE_PYOBJECT)

        iter1 = self.__model.append(None)
        self.__model.set(iter1, 0, 0)
        lang = {}
        for e in engines:
            l = ibus.get_language_name(e.language)
            if l not in lang:
                lang[l] = []
            lang[l].append(e)

        keys = lang.keys()
        keys.sort(locale.strcoll)
        #add "Others" to the end of the combo box
        if ibus.get_language_name("Other") in keys:
            keys.remove(ibus.get_language_name("Other"))
            keys += [ibus.get_language_name("Other")]
        for l in keys:
            iter1 = self.__model.append(None)
            self.__model.set(iter1, 0, l)
            def cmp_engine(a, b):
                if a.rank == b.rank:
                    return locale.strcoll(a.longname, b.longname)
                return int(b.rank - a.rank)
            lang[l].sort(cmp_engine)
            for e in lang[l]:
                iter2 = self.__model.append(iter1)
                self.__model.set(iter2, 0, e)

        self.set_model(self.__model)
        self.set_active(0)

    def __icon_cell_data_cb(self, celllayout, renderer, model, iter):
        engine = self.__model.get_value(iter, 0)

        if isinstance(engine, str) or isinstance (engine, unicode):
            renderer.set_property("visible", False)
            renderer.set_property("sensitive", False)
        elif isinstance(engine, int):
            renderer.set_property("visible", False)
            renderer.set_property("sensitive", False)
        else:
            renderer.set_property("visible", True)
            renderer.set_property("sensitive", True)
            pixbuf = load_icon(engine.icon, gtk.ICON_SIZE_LARGE_TOOLBAR)
            if pixbuf == None:
                pixbuf = load_icon("ibus-engine", gtk.ICON_SIZE_LARGE_TOOLBAR)
            if pixbuf == None:
                pixbuf = load_icon("gtk-missing-image", gtk.ICON_SIZE_LARGE_TOOLBAR)
            renderer.set_property("pixbuf", pixbuf)

    def __name_cell_data_cb(self, celllayout, renderer, model, iter):
        engine = self.__model.get_value(iter, 0)

        if isinstance (engine, str) or isinstance (engine, unicode):
            renderer.set_property("sensitive", False)
            renderer.set_property("text", engine)
            renderer.set_property("weight", pango.WEIGHT_NORMAL)
        elif isinstance(engine, int):
            renderer.set_property("sensitive", True)
            renderer.set_property("text", _("Select an input method"))
            renderer.set_property("weight", pango.WEIGHT_NORMAL)
        else:
            renderer.set_property("sensitive", True)
            renderer.set_property("text", engine.longname)
            renderer.set_property("weight", pango.WEIGHT_BOLD if engine.rank > 0 else pango.WEIGHT_NORMAL)

    def __notify_active_cb(self, combobox, property):
        self.notify("active-engine")

    def do_get_property(self, property):
        if property.name == "active-engine":
            i = self.get_active()
            if i == 0 or i == -1:
                return None
            iter = self.get_active_iter()
            return self.get_model()[iter][0]
        else:
            raise AttributeError, 'unknown property %s' % property.name

    def get_active_engine(self):
        return self.get_property("active-engine")



