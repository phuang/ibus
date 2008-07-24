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
import icon as _icon
from os import path
from ibus import LANGUAGES
from ibus import interface
from languagebar import LanguageBar
from candidatepanel import CandidatePanel

class Panel(ibus.PanelBase):
    def __init__ (self, dbusconn, object_path, __ibus):
        super(Panel, self).__init__(dbusconn, object_path)
        self.__ibus = __ibus
        self.__focus_ic = None

        # add icon search path
        icon_theme = gtk.icon_theme_get_default()
        dir = path.dirname(__file__)
        icondir = path.join(dir, "..", "icons")
        icon_theme.prepend_search_path(icondir)

        self.__language_bar = LanguageBar()
        self.__language_bar.connect("property-activate",
                        lambda widget, prop_name, prop_state: self.property_activate(prop_name, prop_state))
        self.__language_bar.connect("get-im-menu",
                        self.__get_im_menu_cb)
        self.__language_bar.focus_out()
        self.__language_bar.show_all()

        self.__candidate_panel = CandidatePanel()
        self.__candidate_panel.connect("cursor-up",
                        lambda widget: self.cursor_up())
        self.__candidate_panel.connect("cursor-down",
                        lambda widget: self.cursor_down())

        self.__status_icon = gtk.StatusIcon()
        self.__status_icon.connect("popup-menu", self.__status_icon_popup_menu_cb)
        self.__status_icon.connect("activate", self.__status_icon_activate_cb)
        self.__status_icon.set_from_icon_name("engine-default")
        self.__status_icon.set_tooltip("iBus - Running")
        self.__status_icon.set_visible(True)

    def set_cursor_location(self, x, y, w, h):
        self.__candidate_panel.set_cursor_location(x + w, y + h)

    def update_preedit(self, text, attrs, cursor_pos, visible):
        self.__candidate_panel.update_preedit(text, attrs, cursor_pos, visible)

    def show_preedit(self):
        self.__candidate_panel.show_preedit()

    def hide_preedit(self):
        self.__candidate_panel.hide_preedit()

    def update_aux_string(self, text, attrs, visible):
        self.__candidate_panel.update_aux_string(text, attrs, visible)

    def show_aux_string(self):
        self.__candidate_panel.show_aux_string()

    def hide_aux_string(self):
        self.__candidate_panel.hide_aux_string()

    def update_lookup_table(self, lookup_table, visible):
        self.__candidate_panel.update_lookup_table(lookup_table, visible)

    def show_lookup_table(self):
        self.__candidate_panel.show_lookup_table()

    def hide_lookup_table(self):
        self.__candidate_panel.hide_lookup_table()

    def page_up_lookup_table(self):
        self.__candidate_panel.page_up_lookup_table()

    def page_down_lookup_table(self):
        self.__candidate_panel.page_down_lookup_table()

    def cursor_up_lookup_table(self):
        self.__candidate_panel.cursor_up_lookup_table()

    def cursor_down_lookup_table(self):
        self.__candidate_panel.cursor_down_lookup_table()

    def show_candidate_window(self):
        self.__candidate_panel.show()

    def hide_candidate_window(self):
        self.__candidate_panel.hide()

    def show_language_bar(self):
        self.__language_bar.show()

    def hide_language_bar(self):
        self.__language_bar.hide()

    def register_properties(self, props):
        self.__language_bar.register_properties(props)

    def update_property(self, prop):
        self.__language_bar.update_property(prop)

    def __set_im_icon(self, icon_name):
        self.__language_bar.set_im_icon(icon_name)
        if icon_name.startswith("/"):
            self.__status_icon.set_from_file(icon_name)
        else:
            self.__status_icon.set_from_icon_name(icon_name)

    def focus_in(self, ic):
        self.reset()
        self.__focus_ic = ic

        factory, enabled = self.__ibus.GetInputContextStates(ic)

        if factory == "" or not enabled:
            self.__set_im_icon("engine-default")
        else:
            name, lang, icon, authors, credits = self.__ibus.GetFactoryInfo(factory)
            self.__set_im_icon(icon)
        self.__language_bar.focus_in()

    def focus_out(self, ic):
        self.reset()
        if self.__focus_ic == ic:
            self.__focus_ic = None
            self.__language_bar.focus_out()
            self.__set_im_icon("engine-default")

    def states_changed(self):
        if not self.__focus_ic:
            return
        factory, enabled = self.__ibus.GetInputContextStates(self.__focus_ic)
        if enabled == False or not factory:
            self.__set_im_icon("engine-default")
        else:
            name, lang, icon, authors, credits = self.__ibus.GetFactoryInfo(factory)
            self.__set_im_icon(icon)

    def reset(self):
        self.__candidate_panel.reset()
        self.__language_bar.reset()

    def do_destroy(self):
        gtk.main_quit()

    def __create_im_menu(self):
        menu = gtk.Menu()
        factories = self.__ibus.GetFactories()

        if not factories:
            item = gtk.MenuItem(label = "no engine")
            item.set_sensitive(False)
            menu.add(item)
        else:
            tmp = {}
            for factory in factories:
                name, lang, icon, authors, credits = self.__ibus.GetFactoryInfo(factory)
                lang = LANGUAGES.get(lang, "other")
                if not icon:
                    icon = "engine-default"
                if lang not in tmp:
                    tmp[lang] = []
                tmp[lang].append((name, lang, icon, authors, credits, factory))

            langs = tmp.keys()
            langs.sort()
            for lang in langs:
                if len(tmp[lang]) == 1:
                    name, lang, icon, authors, credits, factory = tmp[lang][0]
                    item = gtk.ImageMenuItem("%s - %s" % (lang, name))
                    size = gtk.icon_size_lookup(gtk.ICON_SIZE_MENU)
                    item.set_image (_icon.IconWidget(icon, size[0]))
                    item.connect("activate", self.__menu_item_activate_cb, factory)
                    menu.add(item)
                else:
                    item = gtk.MenuItem(lang)
                    menu.add(item)
                    submenu = gtk.Menu()
                    item.set_submenu(submenu)
                    for name, __lang, icon, authors, credits, factory in tmp[lang]:
                        item = gtk.ImageMenuItem(name)
                        size = gtk.icon_size_lookup(gtk.ICON_SIZE_MENU)
                        item.set_image (_icon.IconWidget(icon, size[0]))
                        item.connect("activate", self.__menu_item_activate_cb, factory)
                        submenu.add(item)


        menu.show_all()
        menu.set_take_focus(False)
        return menu

    def __get_im_menu_cb(self, languagebar):
        menu = self.__create_im_menu()
        return menu

    def __status_icon_activate_cb(self, status_icon):
        if not self.__focus_ic:
            return
        menu = self.__create_im_menu()
        menu.popup(None, None,
                gtk.status_icon_position_menu,
                0,
                gtk.get_current_event_time(),
                self.__status_icon)

    def __status_icon_popup_menu_cb(self, status_icon, button, active_time):
        if not self.__focus_ic:
            return
        menu = self.__create_im_menu()
        menu.popup(None, None,
                gtk.status_icon_position_menu,
                button,
                active_time,
                self.__status_icon)

    def __menu_item_activate_cb(self, item, factory):
        self.__ibus.SetFactory(factory)

gobject.type_register(Panel, "IBusPanel")
