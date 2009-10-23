# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2009 Peng Huang <shawn.p.huang@gmail.com>
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
import ibus
import gettext
from icon import load_icon

_ = lambda a : gettext.dgettext("ibus", a)

class EngineComboBox(gtk.ComboBox):
    def __init__(self, engines):
        super(EngineComboBox, self).__init__()

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
        keys.sort()
        if ibus.get_language_name("Other") in keys:
            keys.remove( ibus.get_language_name("Other"))
            keys += [ibus.get_language_name("Other")]
        for l in keys:
            iter1 = self.__model.append(None)
            self.__model.set(iter1, 0, l)
            for e in lang[l]:
                iter2 = self.__model.append(iter1)
                self.__model.set(iter2, 0, e)

        self.set_model(self.__model)

        renderer = gtk.CellRendererPixbuf()
        renderer.set_property("xalign", 0)
        self.pack_start(renderer, False)
        self.set_cell_data_func(renderer, self.__icon_cell_data_cb)

        renderer = gtk.CellRendererText()
        renderer.set_property("xalign", 0)
        self.pack_start(renderer, True)
        self.set_cell_data_func(renderer, self.__name_cell_data_cb)

        self.set_active(0)

    def __icon_cell_data_cb(self, celllayout, renderer, model, iter):
        engine = self.__model.get_value(iter, 0)

        icon_size = gtk.icon_size_lookup(gtk.ICON_SIZE_LARGE_TOOLBAR)[0]
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
        elif isinstance(engine, int):
            renderer.set_property("sensitive", True)
            renderer.set_property("text", _("Select an input method"))
        else:
            renderer.set_property("sensitive", True)
            renderer.set_property("text", engine.longname)

    def get_active_engine(self):
        i = self.get_active()
        iter = self.get_active_iter()
        if i == 0 or i == -1:
            return None
        return self.get_model()[iter][0]

