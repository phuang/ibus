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

# for python2
from __future__ import print_function

import os
import signal
import sys
import time

from gi.repository import Gio
from gi.repository import GLib
from gi.repository import Gtk
from gi.repository import IBus
from os import path

import keyboardshortcut
import locale
from enginecombobox import EngineComboBox
from enginetreeview import EngineTreeView
from engineabout import EngineAbout
from i18n import DOMAINNAME, _, N_, init as i18n_init

(
    COLUMN_NAME,
    COLUMN_ENABLE,
    COLUMN_PRELOAD,
    COLUMN_VISIBLE,
    COLUMN_ICON,
    COLUMN_DATA,
) = list(range(6))

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
) = list(range(9))

class Setup(object):
    def __flush_gtk_events(self):
        while Gtk.events_pending():
            Gtk.main_iteration()

    def __init__(self):
        super(Setup, self).__init__()

        self.__settings_general = Gio.Settings("org.freedesktop.ibus.general");
        self.__settings_hotkey = Gio.Settings(
                "org.freedesktop.ibus.general.hotkey");
        self.__settings_panel = Gio.Settings("org.freedesktop.ibus.panel");

        # IBus.Bus() calls ibus_bus_new().
        # Gtk.Builder().add_from_file() also calls ibus_bus_new_async()
        # via ibus_im_context_new().
        # Then if IBus.Bus() is called after Gtk.Builder().add_from_file(),
        # the connection delay would be happened without an async
        # finish function.
        self.__bus = None
        self.__init_bus()

        gtk_builder_file = path.join(path.dirname(__file__), "./setup.ui")
        self.__builder = Gtk.Builder()
        self.__builder.set_translation_domain(DOMAINNAME)
        self.__builder.add_from_file(gtk_builder_file);
        self.__init_ui()

    def __init_hotkey(self):
        name = 'triggers'
        label = 'switch_engine'
        shortcuts = self.__settings_hotkey.get_strv(name)
        button = self.__builder.get_object("button_%s" % label)
        entry = self.__builder.get_object("entry_%s" % label)
        entry.set_text("; ".join(shortcuts))
        tooltip = "\n".join(shortcuts)
        tooltip += "\n" + \
            _("Use shortcut with shift to switch to the previous input method") 
        entry.set_tooltip_text(tooltip)
        button.connect("clicked", self.__shortcut_button_clicked_cb,
                name, "general/hotkey", label, entry)

    def __init_panel(self):
        # lookup table orientation
        self.__combobox_lookup_table_orientation = self.__builder.get_object(
                "combobox_lookup_table_orientation")
        self.__settings_panel.bind('lookup-table-orientation',
                                   self.__combobox_lookup_table_orientation,
                                   'active',
                                   Gio.SettingsBindFlags.DEFAULT)

        # auto hide
        self.__combobox_panel_show = self.__builder.get_object(
                "combobox_panel_show")
        self.__settings_panel.bind('show',
                                   self.__combobox_panel_show,
                                   'active',
                                   Gio.SettingsBindFlags.DEFAULT)

        # panel position
        self.__combobox_panel_position = self.__builder.get_object(
                "combobox_panel_position")
        self.__combobox_panel_position.set_active(3)
        #self.__settings_panel.bind('position',
        #                           self.__combobox_panel_position,
        #                           'active',
        #                           Gio.SettingsBindFlags.DEFAULT)

        # custom font
        self.__checkbutton_custom_font = self.__builder.get_object(
                "checkbutton_custom_font")
        self.__settings_panel.bind('use-custom-font',
                                   self.__checkbutton_custom_font,
                                   'active',
                                   Gio.SettingsBindFlags.DEFAULT)

        self.__fontbutton_custom_font = self.__builder.get_object(
                "fontbutton_custom_font")
        self.__settings_panel.bind('custom-font',
                                    self.__fontbutton_custom_font,
                                   'font-name',
                                   Gio.SettingsBindFlags.DEFAULT)
        self.__settings_panel.bind('use-custom-font',
                                    self.__fontbutton_custom_font,
                                   'sensitive',
                                   Gio.SettingsBindFlags.GET)

        # show icon on system tray
        self.__checkbutton_show_icon_on_systray = self.__builder.get_object(
                "checkbutton_show_icon_on_systray")
        self.__settings_panel.bind('show-icon-on-systray',
                                   self.__checkbutton_show_icon_on_systray,
                                   'active',
                                   Gio.SettingsBindFlags.DEFAULT)

        # show ime name
        self.__checkbutton_show_im_name = self.__builder.get_object(
                "checkbutton_show_im_name")
        self.__settings_panel.bind('show-im-name',
                                   self.__checkbutton_show_im_name,
                                   'active',
                                   Gio.SettingsBindFlags.DEFAULT)

    def __init_general(self):
        # embed preedit text
        self.__checkbutton_embed_preedit_text = self.__builder.get_object(
                "checkbutton_embed_preedit_text")
        self.__settings_general.bind('embed-preedit-text',
                                    self.__checkbutton_embed_preedit_text,
                                    'active',
                                    Gio.SettingsBindFlags.DEFAULT)

        # use system keyboard layout setting
        self.__checkbutton_use_sys_layout = self.__builder.get_object(
                "checkbutton_use_sys_layout")
        self.__settings_general.bind('use-system-keyboard-layout',
                                    self.__checkbutton_use_sys_layout,
                                    'active',
                                    Gio.SettingsBindFlags.DEFAULT)

        # use global ime setting
        self.__checkbutton_use_global_engine = self.__builder.get_object(
                "checkbutton_use_global_engine")
        self.__settings_general.bind('use-global-engine',
                                    self.__checkbutton_use_global_engine,
                                    'active',
                                    Gio.SettingsBindFlags.DEFAULT)

        # init engine page
        self.__engines = self.__bus.list_engines()
        self.__combobox = self.__builder.get_object("combobox_engines")
        self.__combobox.set_engines(self.__engines)

        tmp_dict = {}
        for e in self.__engines:
            tmp_dict[e.get_name()] = e
        engine_names = self.__settings_general.get_strv('preload-engines')
        engines = [tmp_dict[name] for name in engine_names if name in tmp_dict]

        self.__treeview = self.__builder.get_object("treeview_engines")
        self.__treeview.set_engines(engines)

        button = self.__builder.get_object("button_engine_add")
        button.connect("clicked", self.__button_engine_add_cb)
        button = self.__builder.get_object("button_engine_remove")
        button.connect("clicked", lambda *args:self.__treeview.remove_engine())
        button = self.__builder.get_object("button_engine_up")
        button.connect("clicked", lambda *args:self.__treeview.move_up_engine())

        button = self.__builder.get_object("button_engine_down")
        button.connect("clicked",
                lambda *args:self.__treeview.move_down_engine())

        button = self.__builder.get_object("button_engine_about")
        button.connect("clicked", self.__button_engine_about_cb)

        self.__engine_setup_exec_list = {}
        button = self.__builder.get_object("button_engine_preferences")
        button.connect("clicked", self.__button_engine_preferences_cb)

        self.__combobox.connect("notify::active-engine",
                self.__combobox_notify_active_engine_cb)
        self.__treeview.connect("notify::active-engine", self.__treeview_notify_cb)
        self.__treeview.connect("notify::engines", self.__treeview_notify_cb)

    def __init_ui(self):
        # add icon search path
        self.__window = self.__builder.get_object("window_preferences")
        self.__window.connect("delete-event", Gtk.main_quit)

        self.__button_close = self.__builder.get_object("button_close")
        self.__button_close.connect("clicked", Gtk.main_quit)

        # auto start ibus
        self.__checkbutton_auto_start = self.__builder.get_object(
                "checkbutton_auto_start")
        self.__checkbutton_auto_start.set_active(self.__is_auto_start())
        self.__checkbutton_auto_start.connect("toggled",
                self.__checkbutton_auto_start_toggled_cb)

        self.__init_hotkey()
        self.__init_panel()
        self.__init_general()

    def __combobox_notify_active_engine_cb(self, combobox, property):
        engine = self.__combobox.get_active_engine()
        button = self.__builder.get_object("button_engine_add")
        button.set_sensitive(
                engine != None and engine not in self.__treeview.get_engines())

    def __get_engine_setup_exec_args(self, engine):
        args = []
        if engine == None:
           return args
        setup = str(engine.get_setup())
        if len(setup) != 0:
            args = setup.split()
            args.insert(1, path.basename(args[0]))
            return args
        name = str(engine.get_name())
        libexecdir = os.environ['IBUS_LIBEXECDIR']
        setup_path = (libexecdir + '/' + 'ibus-setup-' if libexecdir != None \
            else 'ibus-setup-') + name.split(':')[0]
        if path.exists(setup_path):
            args.append(setup_path)
            args.append(path.basename(setup_path))
        return args

    def __treeview_notify_cb(self, treeview, prop):
        if prop.name not in ("active-engine", "engines"):
            return

        engines = self.__treeview.get_engines()
        engine = self.__treeview.get_active_engine()

        self.__builder.get_object("button_engine_remove").set_sensitive(engine != None)
        self.__builder.get_object("button_engine_about").set_sensitive(engine != None)
        self.__builder.get_object("button_engine_up").set_sensitive(engine not in engines[:1])
        self.__builder.get_object("button_engine_down").set_sensitive(engine not in engines[-1:])

        obj = self.__builder.get_object("button_engine_preferences")
        if len(self.__get_engine_setup_exec_args(engine)) != 0:
            obj.set_sensitive(True)
        else:
            obj.set_sensitive(False)

        if prop.name == "engines":
            engine_names = [e.get_name() for e in engines]
            self.__settings_general.set_strv('preload-engines', engine_names)

    def __button_engine_add_cb(self, button):
        engine = self.__combobox.get_active_engine()
        self.__treeview.append_engine(engine)

    def __button_engine_about_cb(self, button):
        engine = self.__treeview.get_active_engine()
        if engine:
            about = EngineAbout(engine)
            about.run()
            about.destroy()

    def __button_engine_preferences_cb(self, button):
        engine = self.__treeview.get_active_engine()
        args = self.__get_engine_setup_exec_args(engine)
        if len(args) == 0:
            return
        name = engine.get_name()
        if name in list(self.__engine_setup_exec_list.keys()):
            try:
                wpid, sts = os.waitpid(self.__engine_setup_exec_list[name],
                                       os.WNOHANG)
                # the setup is still running.
                if wpid == 0:
                    return
            except OSError:
                pass
            del self.__engine_setup_exec_list[name]
        self.__engine_setup_exec_list[name] = os.spawnl(os.P_NOWAIT, *args)

    def __init_bus(self):
        self.__bus = IBus.Bus()
        if self.__bus.is_connected():
            return

        message = _("The IBus daemon is not running. Do you wish to start it?")
        dlg = Gtk.MessageDialog(type = Gtk.MessageType.QUESTION,
                                buttons = Gtk.ButtonsType.YES_NO,
                                text = message)
        id = dlg.run()
        dlg.destroy()
        self.__flush_gtk_events()
        if id != Gtk.ResponseType.YES:
            sys.exit(0)

        main_loop = GLib.MainLoop()

        timeout = 5
        GLib.timeout_add_seconds(timeout, lambda *args: main_loop.quit())
        self.__bus.connect("connected", lambda *args: main_loop.quit())

        os.spawnlp(os.P_NOWAIT, "ibus-daemon", "ibus-daemon", "--xim")

        main_loop.run()

        if self.__bus.is_connected():
            message = _("IBus has been started! "
                "If you cannot use IBus, add the following lines to your $HOME/.bashrc; then relog into your desktop.\n"
                "  export GTK_IM_MODULE=ibus\n"
                "  export XMODIFIERS=@im=ibus\n"
                "  export QT_IM_MODULE=ibus"
                )
            dlg = Gtk.MessageDialog(type = Gtk.MessageType.INFO,
                                    buttons = Gtk.ButtonsType.OK,
                                    text = message)
            id = dlg.run()
            dlg.destroy()
            self.__flush_gtk_events()
        else:
            # Translators: %d == 5 currently
            message = _("IBus daemon could not be started in %d seconds")
            dlg = Gtk.MessageDialog(type = Gtk.MessageType.INFO,
                                    buttons = Gtk.ButtonsType.OK,
                                    text = message % timeout)
            id = dlg.run()
            dlg.destroy()
            self.__flush_gtk_events()
            sys.exit(0)

    def __shortcut_button_clicked_cb(self, button, name, section, _name, entry):
        buttons = (_("_Cancel"), Gtk.ResponseType.CANCEL,
                   _("_OK"), Gtk.ResponseType.OK)
        title = _("Select keyboard shortcut for %s") % \
                _("switching input methods")
        dialog = keyboardshortcut.KeyboardShortcutSelectionDialog(buttons = buttons, title = title)
        text = entry.get_text()
        if text:
            shortcuts = text.split("; ")
        else:
            shortcuts = None
        dialog.set_shortcuts(shortcuts)
        id = dialog.run()
        shortcuts = dialog.get_shortcuts()
        dialog.destroy()
        if id != Gtk.ResponseType.OK:
            return
        self.__settings_hotkey.set_strv(name, shortcuts)
        text = "; ".join(shortcuts)
        entry.set_text(text)
        tooltip = "\n".join(shortcuts)
        tooltip += "\n" + \
            _("Use shortcut with shift to switch to the previous input method") 
        entry.set_tooltip_text(tooltip)

    def __item_started_column_toggled_cb(self, cell, path_str, model):

        # get toggled iter
        iter = model.get_iter_from_string(path_str)
        data = model.get_value(iter, COLUMN_DATA)

        # do something with the value
        if data[DATA_STARTED] == False:
            try:
                self.__bus.register_start_engine(data[DATA_LANG], data[DATA_NAME])
            except Exception as e:
                dlg = Gtk.MessageDialog(type = Gtk.MessageType.ERROR,
                        buttons = Gtk.ButtonsType.CLOSE,
                        message_format = str(e))
                dlg.run()
                dlg.destroy()
                self.__flush_gtk_events()
                return
        else:
            try:
                self.__bus.register_stop_engine(data[DATA_LANG], data[DATA_NAME])
            except Exception as e:
                dlg = Gtk.MessageDialog(type = Gtk.MessageType.ERROR,
                        buttons = Gtk.ButtonsType.CLOSE,
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
                self.__settings_general.set_strv('preload-engines',
                                                 list(self.__preload_engines))
        else:
            if engine in self.__preload_engines:
                self.__preload_engines.remove(engine)
                self.__settings_general.set_strv('preload-engines',
                                                 list(self.__preload_engines))

        # set new value
        model.set(iter, COLUMN_PRELOAD, data[DATA_PRELOAD])

    def __is_auto_start(self):
        link_file = path.join(GLib.get_user_config_dir(),
                              "autostart/ibus.desktop")
        ibus_desktop = path.join(os.getenv("IBUS_PREFIX"),
                                 "share/applications/ibus.desktop")

        if not path.exists(link_file):
            return False
        if not path.islink(link_file):
            return False
        if path.realpath(link_file) != ibus_desktop:
            return False
        return True

    def __checkbutton_auto_start_toggled_cb(self, button):
        auto_start_dir = path.join(GLib.get_user_config_dir(), "autostart")
        if not path.isdir(auto_start_dir):
            os.makedirs(auto_start_dir)

        link_file = path.join(GLib.get_user_config_dir(),
                              "autostart/ibus.desktop")
        ibus_desktop = path.join(os.getenv("IBUS_PREFIX"),
                                 "share/applications/ibus.desktop")
        # unlink file
        try:
            os.unlink(link_file)
        except:
            pass
        if self.__checkbutton_auto_start.get_active():
            os.symlink(ibus_desktop, link_file)

    def __sigusr1_cb(self, *args):
        self.__window.present()

    def run(self):
        self.__window.show_all()
        signal.signal(signal.SIGUSR1, self.__sigusr1_cb)
        Gtk.main()

if __name__ == "__main__":
    try:
        locale.setlocale(locale.LC_ALL, '')
    except locale.Error:
        print("Using the fallback 'C' locale", file=sys.stderr)
        locale.setlocale(locale.LC_ALL, 'C')

    i18n_init()
    setup = Setup()
    setup.run()
