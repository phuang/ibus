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

import os
import sys
from os import path
import time
from xdg import BaseDirectory
import gtk
import gobject
import ibus
from gtk import gdk, glade
import keyboardshortcut

from gettext import dgettext
_  = lambda a : dgettext("ibus", a)
N_ = lambda a : a

(
    COLUMN_NAME,
    COLUMN_ENABLE,
    COLUMN_PRELOAD,
    COLUMN_VISIBLE,
    COLUMN_ICON,
    COLUMN_DATA,
) = range(6)

(
    DATA_NAME,
    DATA_LOCAL_NAME,
    DATA_LANG,
    DATA_ICON,
    DATA_AUTHOR,
    DATA_CREDITS,
    DATA_EXEC,
    DATA_STARTED,
    DATA_PRELOAD
) = range(9)

CONFIG_GENERAL_SHORTCUT = "/general/keyboard_shortcut_%s"
CONFIG_PRELOAD_ENGINES = "/general/preload_engines"
CONFIG_PANEL_LOOKUP_TABLE_ORIENTATION = "/panel/lookup_table_orientation"
CONFIG_PANEL_AUTO_HIDE = "/panel/auto_hide"
CONFIG_PANEL_USE_CUSTOM_FONT = "/panel/use_custom_font"
CONFIG_PANEL_CUSTOM_FONT = "/panel/custom_font"

class Setup(object):
    def __flush_gtk_events(self):
        while gtk.events_pending():
            gtk.main_iteration()

    def __init__(self):
        super(Setup, self).__init__()
        glade.textdomain("ibus")
        glade_file = path.join(path.dirname(__file__), "./setup.glade")
        self.__xml = glade.XML(glade_file)
        self.__bus = None
        self.__init_bus()
        self.__init_ui()

    def __init_ui(self):
        # add icon search path
        icon_theme = gtk.icon_theme_get_default()
        dir = path.dirname(__file__)
        icondir = path.join(dir, "..", "icons")
        icon_theme.prepend_search_path(icondir)


        self.__dialog = self.__xml.get_widget("dialog_setup")

        # auto start ibus
        self.__checkbutton_auto_start = self.__xml.get_widget("checkbutton_auto_start")
        self.__checkbutton_auto_start.set_active(self.__is_auto_start())
        self.__checkbutton_auto_start.connect("toggled", self.__checkbutton_auto_start_toggled_cb)

        # keyboard shortcut
        for name in (N_("trigger"), N_("next_engine"), N_("prev_engine")):
            shortcuts = self.__bus.config_get_value(CONFIG_GENERAL_SHORTCUT % name, [])
            button = self.__xml.get_widget("button_%s" % name)
            entry = self.__xml.get_widget("entry_%s" % name)
            entry.set_text(";".join(shortcuts))
            button.connect("clicked", self.__shortcut_button_clicked_cb, name, entry)

        # lookup table orientation
        self.__combobox_lookup_table_orientation = self.__xml.get_widget("combobox_lookup_table_orientation")
        self.__combobox_lookup_table_orientation.set_active(
            self.__bus.config_get_value(CONFIG_PANEL_LOOKUP_TABLE_ORIENTATION, 0))
        self.__combobox_lookup_table_orientation.connect("changed",
            self.__combobox_lookup_table_orientation_changed_cb)

        # auto hide
        self.__checkbutton_auto_hide = self.__xml.get_widget("checkbutton_auto_hide")
        self.__checkbutton_auto_hide.set_active(
            self.__bus.config_get_value(CONFIG_PANEL_AUTO_HIDE, False))
        self.__checkbutton_auto_hide.connect("toggled", self.__checkbutton_auto_hide_toggled_cb)

        # custom font
        self.__checkbutton_custom_font = self.__xml.get_widget("checkbutton_custom_font")
        self.__checkbutton_custom_font.set_active(
            self.__bus.config_get_value(CONFIG_PANEL_USE_CUSTOM_FONT, False))
        self.__checkbutton_custom_font.connect("toggled", self.__checkbutton_custom_font_toggled_cb)

        self.__fontbutton_custom_font = self.__xml.get_widget("fontbutton_custom_font")
        if self.__bus.config_get_value(CONFIG_PANEL_USE_CUSTOM_FONT, False):
            self.__fontbutton_custom_font.set_sensitive(True)
        else:
            self.__fontbutton_custom_font.set_sensitive(False)
        font_name = gtk.settings_get_default().get_property("gtk-font-name")
        font_name = unicode(font_name, "utf-8")
        font_name = self.__bus.config_get_value(CONFIG_PANEL_CUSTOM_FONT, font_name)
        self.__fontbutton_custom_font.connect("notify::font-name", self.__fontbutton_custom_font_notify_cb)
        self.__fontbutton_custom_font.set_font_name(font_name)

        self.__init_engine_view()

    def __init_bus(self):
        try:
            self.__bus = ibus.Bus()
            self.__bus.connect("config-value-changed", self.__config_value_changed_cb)
            self.__bus.connect("config-reloaded", self.__config_reloaded_cb)
            self.__bus.config_add_watch("/general")
            self.__bus.config_add_watch("/panel")
        except:
            while self.__bus == None:
                message = _("IBus daemon is not started. Do you want to start it now?")
                print >> sys.stderr, message
                dlg = gtk.MessageDialog(type = gtk.MESSAGE_QUESTION,
                        buttons = gtk.BUTTONS_YES_NO,
                        message_format = message)
                id = dlg.run()
                dlg.destroy()
                self.__flush_gtk_events()
                if id != gtk.RESPONSE_YES:
                    sys.exit(0)
                pid = os.spawnlp(os.P_NOWAIT, "ibus", "ibus")
                time.sleep(1)
                try:
                    self.__bus = ibus.Bus()
                except:
                    continue
                message = _("IBus has been started! " + \
                    "If you can not use IBus, please add below lines in $HOME/.bashrc, and relogin your desktop.\n" + \
                    "  export GTK_IM_MODULE=ibus\n" + \
                    "  export XMODIFIERS=@im=ibus\n"
                    "  export QT_IM_MODULE=ibus"
                    )
                dlg = gtk.MessageDialog(type = gtk.MESSAGE_INFO,
                                        buttons = gtk.BUTTONS_OK,
                                        message_format = message)
                id = dlg.run()
                dlg.destroy()
                self.__flush_gtk_events()

    def __init_engine_view(self):
        # engines tree
        self.__tree = self.__xml.get_widget("treeview_engines")
        self.__preload_engines = set(self.__bus.config_get_value(CONFIG_PRELOAD_ENGINES, []))
        model = self.__create_model()
        self.__tree.set_model(model)

        # column for engine
        column = gtk.TreeViewColumn()
        column.set_title(_("Engine"))

        renderer = gtk.CellRendererPixbuf()
        renderer.set_property("xalign", 0.5)

        column.pack_start(renderer)
        column.set_attributes(renderer, pixbuf = COLUMN_ICON, visible = COLUMN_VISIBLE)

        renderer = gtk.CellRendererText()
        renderer.set_property("xalign", 0.0)

        # column.set_clickable(True)
        column.pack_start(renderer)
        column.set_attributes(renderer, text = COLUMN_NAME)

        self.__tree.append_column(column)

        # column for started
        renderer = gtk.CellRendererToggle()
        renderer.set_data('column', COLUMN_ENABLE)
        renderer.set_property("xalign", 0.5)
        renderer.connect("toggled", self.__item_started_column_toggled_cb, model)

        #col_offset = gtk.TreeViewColumn("Holiday", renderer, text=HOLIDAY_NAME)
        column = gtk.TreeViewColumn(_("Started"), renderer, active = COLUMN_ENABLE, visible = COLUMN_VISIBLE)
        self.__tree.append_column(column)

        # column for preload
        renderer = gtk.CellRendererToggle()
        renderer.set_data('column', COLUMN_PRELOAD)
        renderer.set_property("xalign", 0.5)
        renderer.connect("toggled", self.__item_preload_column_toggled_cb, model)

        column = gtk.TreeViewColumn(_("Preload"), renderer, active = COLUMN_PRELOAD, visible = COLUMN_VISIBLE)
        self.__tree.append_column(column)

        renderer = gtk.CellRendererText()
        column = gtk.TreeViewColumn("", renderer)
        self.__tree.append_column(column)

    def __shortcut_button_clicked_cb(self, button, name, entry):
        buttons = (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OK, gtk.RESPONSE_OK)
        title = _("Select keyboard shortcut for %s") %  _(name)
        dialog = keyboardshortcut.KeyboardShortcutSelectionDialog(buttons = buttons, title = title)
        text = entry.get_text()
        if text:
            shortcuts = text.split(';')
        else:
            shortcuts = None
        dialog.set_shortcuts(shortcuts)
        id = dialog.run()
        shortcuts = list(set(dialog.get_shortcuts()))
        dialog.destroy()
        if id != gtk.RESPONSE_OK:
            return
        self.__bus.config_set_value(CONFIG_GENERAL_SHORTCUT % name, shortcuts)
        entry.set_text(";".join(shortcuts))


    def __item_started_column_toggled_cb(self, cell, path_str, model):

        # get toggled iter
        iter = model.get_iter_from_string(path_str)
        data = model.get_value(iter, COLUMN_DATA)

        # do something with the value
        if data[DATA_STARTED] == False:
            try:
                self.__bus.register_start_engine(data[DATA_LANG], data[DATA_NAME])
            except Exception, e:
                dlg = gtk.MessageDialog(type = gtk.MESSAGE_ERROR,
                        buttons = gtk.BUTTONS_CLOSE,
                        message_format = str(e))
                dlg.run()
                dlg.destroy()
                self.__flush_gtk_events()
                return
        else:
            try:
                self.__bus.register_stop_engine(data[DATA_LANG], data[DATA_NAME])
            except Exception, e:
                dlg = gtk.MessageDialog(type = gtk.MESSAGE_ERROR,
                        buttons = gtk.BUTTONS_CLOSE,
                        message_format = str(e))
                dlg.run()
                dlg.destroy()
                self.__flush_gtk_events()
                return
        data[DATA_STARTED] = not data[DATA_STARTED]

        # set new value
        model.set(iter, COLUMN_ENABLE, data[DATA_STARTED])

    def __item_preload_column_toggled_cb(self, cell, path_str, model):

        # get toggled iter
        iter = model.get_iter_from_string(path_str)
        data = model.get_value(iter, COLUMN_DATA)

        data[DATA_PRELOAD] = not data[DATA_PRELOAD]
        engine = "%s:%s" % (data[DATA_LANG], data[DATA_NAME])

        if data[DATA_PRELOAD]:
            if engine not in self.__preload_engines:
                self.__preload_engines.add(engine)
                self.__bus.config_set_list(CONFIG_PRELOAD_ENGINES, list(self.__preload_engines), "s")
        else:
            if engine in self.__preload_engines:
                self.__preload_engines.remove(engine)
                self.__bus.config_set_list(CONFIG_PRELOAD_ENGINES, list(self.__preload_engines), "s")


        # set new value
        model.set(iter, COLUMN_PRELOAD, data[DATA_PRELOAD])

    def __load_icon(self, icon, icon_size):
        pixbuf = None
        try:
            pixbuf = gdk.pixbuf_new_from_file(icon)
            w, h = pixbuf.get_width(), pixbuf.get_height()
            rate = max(w, h) / float(icon_size)
            w = int(w / rate)
            h = int(h / rate)
            pixbuf = pixbuf.scale_simple(w, h, gdk.INTERP_BILINEAR)
        except:
            pass
        if pixbuf == None:
            try:
                theme = gtk.icon_theme_get_default()
                pixbuf = theme.load_icon(icon, icon_size, 0)
            except:
                pass
        return pixbuf

    def __create_model(self):
        # create tree store
        model = gtk.TreeStore(
            gobject.TYPE_STRING,
            gobject.TYPE_BOOLEAN,
            gobject.TYPE_BOOLEAN,
            gobject.TYPE_BOOLEAN,
            gdk.Pixbuf,
            gobject.TYPE_PYOBJECT)

        langs = dict()

        self.__bus.register_reload_engines()
        for name, local_name, lang, icon, author, credits, _exec, started in self.__bus.register_list_engines():
            _lang = ibus.LANGUAGES.get(lang, "other")
            _lang = _(_lang)
            if _lang not in langs:
                langs[_lang] = list()
            langs[_lang].append([name, local_name, lang, icon, author, credits, _exec, started])

        keys = langs.keys()
        keys.sort()

        icon_size = gtk.icon_size_lookup(gtk.ICON_SIZE_LARGE_TOOLBAR)[0]
        pixbuf_missing = self.__load_icon("engine-default", icon_size)
        if pixbuf_missing == None:
            pixbuf_missing = self.__load_icon("gtk-missing-image", icon_size)

        for key in keys:
            iter = model.append(None)
            model.set(iter,
                COLUMN_NAME, key,
                COLUMN_ENABLE, False,
                COLUMN_PRELOAD, False,
                COLUMN_VISIBLE, False,
                COLUMN_ICON, None,
                COLUMN_DATA, None)
            langs[key].sort()

            for name, local_name, lang, icon, author, credits, _exec, started in langs[key]:
                child_iter = model.append(iter)
                is_preload = "%s:%s" % (lang, name) in self.__preload_engines

                pixbuf = self.__load_icon(icon, icon_size)
                if pixbuf == None:
                    pixbuf = pixbuf_missing

                model.set(child_iter,
                    COLUMN_NAME, local_name,
                    COLUMN_ENABLE, started,
                    COLUMN_PRELOAD, is_preload,
                    COLUMN_VISIBLE, True,
                    COLUMN_ICON, pixbuf,
                    COLUMN_DATA,
                    [name, local_name, lang, icon, author, credits, _exec, started, is_preload])

        return model

    def __is_auto_start(self):
        link_file = path.join(BaseDirectory.xdg_config_home, "autostart/ibus.desktop")
        ibus_desktop = path.join(os.getenv("IBUS_PREFIX"), "share/applications/ibus.desktop")

        if not path.exists(link_file):
            return False
        if not path.islink(link_file):
            return False
        if path.realpath(link_file) != ibus_desktop:
            return False
        return True

    def __checkbutton_auto_start_toggled_cb(self, button):
        auto_start_dir = path.join(BaseDirectory.xdg_config_home, "autostart")
        if not path.isdir(auto_start_dir):
            os.makedirs(auto_start_dir)

        link_file = path.join(BaseDirectory.xdg_config_home, "autostart/ibus.desktop")
        ibus_desktop = path.join(os.getenv("IBUS_PREFIX"), "share/applications/ibus.desktop")
        # unlink file
        try:
            os.unlink(link_file)
        except:
            pass
        if self.__checkbutton_auto_start.get_active():
            os.symlink(ibus_desktop, link_file)

    def __combobox_lookup_table_orientation_changed_cb(self, combobox):
        self.__bus.config_set_value(
            CONFIG_PANEL_LOOKUP_TABLE_ORIENTATION,
            self.__combobox_lookup_table_orientation.get_active())

    def __checkbutton_auto_hide_toggled_cb(self, button):
        self.__bus.config_set_value(
            CONFIG_PANEL_AUTO_HIDE,
            self.__checkbutton_auto_hide.get_active())

    def __checkbutton_custom_font_toggled_cb(self, button):
        if self.__checkbutton_custom_font.get_active():
            self.__fontbutton_custom_font.set_sensitive(True)
            self.__bus.config_set_value(CONFIG_PANEL_USE_CUSTOM_FONT, True)
        else:
            self.__fontbutton_custom_font.set_sensitive(False)
            self.__bus.config_set_value(CONFIG_PANEL_USE_CUSTOM_FONT, False)

    def __fontbutton_custom_font_notify_cb(self, button, arg):
        font_name = self.__fontbutton_custom_font.get_font_name()
        font_name = unicode(font_name, "utf-8")
        self.__bus.config_set_value(CONFIG_PANEL_CUSTOM_FONT, font_name)

    def __config_value_changed_cb(self, bus, key, value):
        if key == CONFIG_PANEL_LOOKUP_TABLE_ORIENTATION:
            item = self.__bus.config_get_value(CONFIG_PANEL_LOOKUP_TABLE_ORIENTATION, 0)
            if item != 0 and item != 1:
                item = 0
            self.__combobox_lookup_table_orientation.set_active(item)

    def __config_reloaded_cb(self, bus):
        pass

    def run(self):
        return self.__dialog.run()

if __name__ == "__main__":
    Setup().run()
