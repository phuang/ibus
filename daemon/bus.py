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

import sys
import ibus
import gobject
from ibus import keysyms
from ibus import modifier
from contextmanager import ContextManager
from factorymanager import FactoryManager
from panel import Panel, DummyPanel
from notifications import Notifications, DummyNotifications
from config import Config, DummyConfig
from register import Register
import _dbus

from gettext import dgettext
_  = lambda a : dgettext("ibus", a)
N_ = lambda a : a

class IBus(ibus.Object):
    def __init__(self):
        super(IBus, self).__init__()
        _dbus.bus.connect("name-owner-changed", self.__dbus_name_owner_changed_cb)
        self.__context_manager = ContextManager()
        self.__factory_manager = FactoryManager()
        self.__factory_manager.connect("default-factory-changed",
                self.__factory_manager_default_factory_changed_cb)

        self.__panel = DummyPanel()
        self.__panel_handlers = list()
        self.__install_panel_handlers()

        self.__notifications = DummyNotifications()
        self.__notifications_handlers = list()
        self.__install_notifications_handlers()

        self.__no_engine_notification_id = 0
        self.__no_engine_notification_show = True

        self.__config = DummyConfig()

        self.__register = Register()

        self.__focused_context = None
        self.__context_handlers = list()

        self.__connections = list()

        self.__prev_key = None
        self.__config_load_settings ()

    def __config_load_settings (self):
        self.__shortcut_trigger = self.__load_config_shortcut(
                "general", "keyboard_shortcut_trigger",
                ibus.CONFIG_GENERAL_SHORTCUT_TRIGGER_DEFAULT)
        self.__shortcut_next_engine = self.__load_config_shortcut(
                "general", "keyboard_shortcut_next_engine",
                ibus.CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE_DEFAULT)
        self.__shortcut_prev_engine = self.__load_config_shortcut(
                "general", "keyboard_shortcut_next_engine",
                ibus.CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE_DEFAULT)
        self.__default_factory = None

    def __dbus_name_owner_changed_cb(self, bus, name, old_name, new_name):
        if name == ibus.IBUS_SERVICE_PANEL:
            self.__panel_changed(new_name)
        elif name == ibus.IBUS_SERVICE_CONFIG:
            self.__config_changed(new_name)
        elif name == ibus.IBUS_SERVICE_NOTIFICATIONS:
            self.__notifications_changed(new_name)

    def __factory_manager_default_factory_changed_cb(self, manager, factory):
        if self.__default_factory != factory:
            self.__default_factory = factory
        if factory == None:
            return
        self.__config.set_value("general", "default_factory", factory.get_object_path())

    def __load_config_shortcut(self, section, name, default_value):

        # load trigger
        shortcut_strings = default_value
        try:
            shortcut_strings = self.__config.get_value(section, name)
        except:
            pass
        shortcuts = []
        for s in shortcut_strings:
            keyval, keymask = self.__parse_shortcut_string(s)
            if keyval != 0:
                shortcuts.append((keyval, keymask))
        return shortcuts

    def new_connection(self, conn):
        IBusProxy(self, conn)
        conn.connect("destroy", self.__conn_destroy_cb)
        self.__connections.append(conn)

    def __conn_destroy_cb(self, conn):
        self.__connections.remove(conn)

    ##########################################################
    # methods for im context
    ##########################################################
    def __load_default_factory(self):
        if self.__default_factory != None:
            return
        try:
            factory_path = self.__config.get_value("general", "default_factory")
            self.__default_factory = self.__factory_manager.get_factory(factory_path)
        except:
            pass
        if self.__default_factory != None:
            self.__factory_manager.set_default_factory(self.__default_factory)
            return

    def create_input_context(self, name, conn):
        context = self.__context_manager.create_input_context(name, conn)
        self.__install_context_handlers(context)
        
        self.__load_default_factory()
        if self.__default_factory != None:
            engine = self.__default_factory.create_engine()
            context.set_engine(engine)
        return context.get_path()

    def release_input_context(self, ic, conn):
        self.__context_manager.release_input_context(ic, conn)

    def focus_in(self, context):
        if self.__focused_context != None:
            if context == None:
                return
            self.__focused_context.focus_out()
        
        self.__focused_context = context
        self.__panel.focus_in(context.get_path())

    def focus_out(self, context):
        if context == self.__focused_context:
            self.__remove_focused_context_handlers()
            self.__focused_context = None
            self.__panel.focus_out(context.get_path())

    def is_enabled(self, ic, conn):
        context = self.__lookup_context(ic, conn)
        return context.is_enabled()

    def set_capabilities(self, ic, caps, conn):
        context = self.__lookup_context(ic, conn)
        return context.set_capabilities(caps)

    def process_key_event(self, ic, keyval, is_press, state,
                                conn, reply_cb, error_cb):
        context = self.__lookup_context(ic, conn)

        # focus in the context, if context supports focus
        if context != self.__focused_context and context.get_support_focus():
            self.focus_in(ic, conn)

        if self.__filter_keyboard_shortcuts(context, keyval, is_press, state):
            reply_cb(True)
            return
        else:
            context.process_key_event(keyval, is_press, state, reply_cb, error_cb)

    def __set_cursor_location_cb(self, context, x, y, w, h):
        if context != self.__focused_context:
            return
        self.__panel.set_cursor_location(x, y, w, h)

    def __context_enable(self, context):
        if context.get_engine() == None:
            self.__load_default_factory()
            if self.__default_factory == None:
                self.__default_factory = self.__factory_manager.get_default_factory()
            if self.__default_factory:
                engine = self.__default_factory.create_engine()
                engine.focus_in()
                context.set_engine(engine)
            else:
                if self.__no_engine_notification_show:
                    self.__no_engine_notification_id = self.__notifications.notify(
                            self.__no_engine_notification_id,
                            "ibus", _("Cannot enable input engine"),
                            _("IBus can not enable input engine, because IBus does not load any input engines!\n"
                                "Please use ibus-setup program to load some input engines."),
                            ["Setup", _("Setup"), "NoAgain", _("Don't show this again")],
                            15000)

        if context.get_engine() != None:
            context.set_enable(True)
        self.__panel.states_changed()

    def __context_disable(self, context):
        context.set_enable(False)
        self.__panel.reset()
        self.__panel.states_changed()

    def __context_next_factory(self, context):
        old_factory = context.get_factory()
        new_factory = self.__factory_manager.get_next_factory(old_factory)
        self.__factory_manager.set_default_factory(new_factory)
        engine = new_factory.create_engine()
        self.__panel.reset()
        engine.focus_in()
        context.set_engine(engine)
        self.__panel.states_changed()

    def __match_keyboard_shortcuts(self, keyval, is_press, state, shortcuts):
        for sc in shortcuts:
            if state == sc[1] and keyval == sc[0]:
                if state & modifier.RELEASE_MASK == 0:
                    return True
                if self.__prev_key[0] == keyval and \
                    self.__prev_key[1] == True:
                    return True
        return False

    def __filter_keyboard_shortcuts(self, context, keyval, is_press, state):
        state = state & (modifier.CONTROL_MASK | \
            modifier.SHIFT_MASK | \
            modifier.MOD1_MASK | \
            modifier.SUPER_MASK | \
            modifier.HYPER_MASK | \
            modifier.META_MASK)
        if not is_press:
            state = state | modifier.RELEASE_MASK

        retval = True
        if self.__match_keyboard_shortcuts(keyval,
            is_press, state, self.__shortcut_trigger):
            if context.is_enabled():
                self.__context_disable(context)
            else:
                self.__context_enable(context)
            retval = True
        elif self.__match_keyboard_shortcuts(keyval,
            is_press, state, self.__shortcut_next_engine):
            if not context.is_enabled():
                self.__context_enable(context)
            else:
                self.__context_next_factory(context)
        else:
            retval = False

        self.__prev_key = (keyval, is_press, state)
        return retval

    def __lookup_context(self, ic, conn):
        return self.__context_manager.lookup_context(ic, conn)

    def __install_context_handlers(self, context):
        # Install all callback functions
        signals = (
            ("focus-in", lambda c: self.focus_in(c)),
            ("focus-out", lambda c: self.focus_out(c)),
            ("set-cursor-location", self.__set_cursor_location_cb),
            ("update-preedit", self.__update_preedit_cb),
            ("show-preedit", self.__show_preedit_cb),
            ("hide-preedit", self.__hide_preedit_cb),
            ("update-aux-string", self.__update_aux_string_cb),
            ("show-aux-string", self.__show_aux_string_cb),
            ("hide-aux-string", self.__hide_aux_string_cb),
            ("update-lookup-table", self.__update_lookup_table_cb),
            ("show-lookup-table", self.__show_lookup_table_cb),
            ("hide-lookup-table", self.__hide_lookup_table_cb),
            ("page-up-lookup-table", self.__page_up_lookup_table_cb),
            ("page-down-lookup-table", self.__page_down_lookup_table_cb),
            ("cursor-up-lookup-table", self.__cursor_up_lookup_table_cb),
            ("cursor-down-lookup-table", self.__cursor_down_lookup_table_cb),
            ("register-properties", self.__register_properties_cb),
            ("update-property", self.__update_property_cb),
            ("engine-lost", self.__engine_lost_cb),
            ("destroy", self.__context_destroy_cb)
        )
        for name, handler in signals:
            context.connect(name, handler)

    def __remove_focused_context_handlers(self):
        if self.__focused_context == None:
            return
        map(self.__focused_context.disconnect, self.__context_handlers)
        self.__context_handlers = []

    def __update_preedit_cb(self, context, text, attrs, cursor_pos, visible):
        if self.__focused_context != context:
            return
        self.__panel.update_preedit(text, attrs, cursor_pos, visible)

    def __show_preedit_cb(self, context):
        if self.__focused_context != context:
            return
        self.__panel.show_preedit()

    def __hide_preedit_cb(self, context):
        if self.__focused_context != context:
            return
        self.__panel.hide_preedit()

    def __update_aux_string_cb(self, context, text, attrs, visible):
        if self.__focused_context != context:
            return
        self.__panel.update_aux_string(text, attrs, visible)

    def __show_aux_string_cb(self, context):
        if self.__focused_context != context:
            return
        self.__panel.show_aux_string()

    def __hide_aux_string_cb(self, context):
        if self.__focused_context != context:
            return
        self.__panel.hide_aux_string()

    def __update_lookup_table_cb(self, context, lookup_table, visible):
        if self.__focused_context != context:
            return
        self.__panel.update_lookup_table(lookup_table, visible)

    def __show_lookup_table_cb(self, context):
        if self.__focused_context != context:
            return
        self.__panel.show_lookup_table()

    def __hide_lookup_table_cb(self, context):
        if self.__focused_context != context:
            return
        self.__panel.hide_lookup_table()

    def __page_up_lookup_table_cb(self, context):
        if self.__focused_context != context:
            return
        self.__panel.page_up_lookup_table()

    def __page_down_lookup_table_cb(self, context):
        if self.__focused_context != context:
            return
        self.__panel.page_down_lookup_table()

    def __cursor_up_lookup_table_cb(self, context):
        if self.__focused_context != context:
            return
        self.__panel.cursor_up_lookup_table()

    def __cursor_down_lookup_table_cb(self, context):
        if self.__focused_context != context:
            return
        self.__panel.cursor_down_lookup_table()

    def __register_properties_cb(self, context, props):
        if self.__focused_context != context:
            return
        self.__panel.register_properties(props)


    def __update_property_cb(self, context, prop):
        if self.__focused_context != context:
            return
        self.__panel.update_property(prop)

    def __engine_lost_cb(self, context):
        if self.__focused_context != context:
            return
        self.__panel.reset()

    def __context_destroy_cb(self, context):
        if context != self.__focused_context:
            return
        self.__panel.focus_out(context.get_path())
        self.__focused_context = None

    ##########################################################
    # methods for im engines
    ##########################################################
    def register_factories(self, object_paths, conn):
        self.__factory_manager.register_factories(object_paths, conn)


    ##########################################################
    # methods for panel
    ##########################################################
    def __panel_changed(self, bus_name):
        if not isinstance(self.__panel, DummyPanel):
            self.__uninstall_panel_handlers()
            self.__panel.destroy()
        ibusconn = _dbus.bus.get_connection_by_name(bus_name)
        if ibusconn == None:
            self.__panel = DummyPanel()
        else:
            self.__panel = Panel(ibusconn)
        self.__install_panel_handlers()
        if self.__focused_context:
            self.__panel.focus_in(self.__focused_context.get_id())

    def __install_panel_handlers(self):
        signals = (
            ("page-up", self.__panel_page_up_cb),
            ("page-down", self.__panel_page_down_cb),
            ("cursor-up", self.__panel_cursor_up_cb),
            ("cursor-down", self.__panel_cursor_down_cb),
            ("property-activate", self.__panel_property_active_cb),
            ("property-show", self.__panel_property_show_cb),
            ("property-hide", self.__panel_property_hide_cb),
            ("destroy", self.__panel_destroy_cb)
        )

        for signal, handler in signals:
            id = self.__panel.connect(signal, handler)
            self.__panel_handlers.append(id)

    def __uninstall_panel_handlers(self):
        map(lambda id:self.__panel.disconnect(id), self.__panel_handlers)
        self.__panel_handlers = list()


    def __panel_page_up_cb(self, panel):
        assert panel == self.__panel
        if self.__focused_context:
            self.__focused_context.page_up()

    def __panel_page_down_cb(self, panel):
        assert panel == self.__panel
        if self.__focused_context:
            self.__focused_context.page_down()

    def __panel_cursor_up_cb(self, panel):
        assert panel == self.__panel
        if self.__focused_context:
            self.__focused_context.cursor_up()

    def __panel_cursor_down_cb(self, panel):
        assert panel == self.__panel
        if self.__focused_context:
            self.__focused_context.cursor_down()

    def __panel_property_active_cb(self, panel, prop_name, prop_state):
        assert panel == self.__panel
        if self.__focused_context:
            self.__focused_context.property_activate(prop_name, prop_state)

    def __panel_property_show_cb(self, panel, prop_name):
        assert panel == self.__panel
        if self.__focused_context:
            self.__focused_context.property_show(prop_name)

    def __panel_property_hide_cb(self, panel, prop_name):
        assert panel == self.__panel
        if self.__focused_context:
            self.__focused_context.property_hide(prop_name)

    def __panel_destroy_cb(self, panel):
        if panel == self.__panel:
            self.__uninstall_panel_handlers()
            self.__panel = DummyPanel()
    
    ##########################################################
    # methods for notifications
    ##########################################################
    def __notifications_changed(self, bus_name):
        if not isinstance(self.__notifications, DummyNotifications):
            self.__uninstall_notifications_handlers()
            self.__notifications.destroy()
        ibusconn = _dbus.bus.get_connection_by_name(bus_name)
        if ibusconn == None:
            self.__notifications = DummyNotifications()
        else:
            self.__notifications = Notifications(ibusconn)
        self.__install_notifications_handlers()

    def __install_notifications_handlers(self):
        signals = (
            ("notification-closed", self.__notifications_notification_closed_cb),
            ("action-invoked", self.__notifications_action_invoked_cb),
            ("destroy", self.__notifications_destroy_cb)
        )

        for signal, handler in signals:
            id = self.__notifications.connect(signal, handler)
            self.__notifications_handlers.append(id)

    def __uninstall_notifications_handlers(self):
        map(lambda id:self.__notifications.disconnect(id), self.__notifications_handlers)
        self.__notifications_handlers = list()

    def __notifications_notification_closed_cb(self, notifications, id, reason):
        pass

    def __notifications_action_invoked_cb(self, notifications, id, action_key):
        if id == self.__no_engine_notification_id:
            if action_key == "NoAgain":
                self.__no_engine_notification_show = False
            elif action_key == "Setup":
                self.__panel.start_setup()


    def __notifications_destroy_cb(self, notifications):
        if notifications == self.__notifications:
            self.__uninstall_notifications_handlers()
            self.__notifications = DummyNotifications()


    ##########################################################
    # methods for config
    ##########################################################
    def __config_changed(self, bus_name):
        ibusconn = _dbus.bus.get_connection_by_name(bus_name)
        self.__config.destroy()
        self.__config = Config(ibusconn)
        self.__config.connect("value-changed", self.__config_value_changed_cb)
        gobject.idle_add (self.__config_load_settings)

    def __parse_shortcut_string(self, string):
        keys = string.split("+")
        keymask = 0
        for name, mask in modifier.MODIFIER_NAME_TABLE:
            if name in keys[:-1]:
                keymask |= mask
            keyname = keys[-1]
            keyval = keysyms.name_to_keycode(keyname)
            if keyval == None:
                keyval = 0

        return keyval, keymask

    def __config_value_changed_cb (self, config, section, name, value):
        if section == "general":
            if name == "keyboard_shortcut_trigger":
                self.__shortcut_trigger = self.__load_config_shortcut(
                        "general", "keyboard_shortcut_trigger",
                        ibus.CONFIG_GENERAL_SHORTCUT_TRIGGER_DEFAULT)
            elif name =="keyboard_shortcut_next_engine":
                self.__shortcut_next_engine = self.__load_config_shortcut(
                        "general", "keyboard_shortcut_next_engine",
                        ibus.CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE_DEFAULT)

    def __config_destroy_cb(self, config):
        if config == self.__config:
            self.__config = DefaultConfig()

    ##########################################################
    # engine register methods
    ##########################################################
    def register_list_engines(self, conn):
        return self.__register.list_engines()

    def register_reload_engines(self, conn):
        return self.__register.reload_engines()

    def register_start_engine(self, lang, name, conn):
        return self.__register.start_engine(lang, name)

    def register_restart_engine(self, lang, name, conn):
        return self.__register.restart_engine(lang, name)

    def register_stop_engine(self, lang, name, conn):
        return self.__register.stop_engine(lang, name)

    ##########################################################
    # general methods
    ##########################################################
    def get_factories(self):
        return self.__factory_manager.get_factories()

    def get_factory_info(self, factory_path):
        return self.__factory_manager.get_factory_info(factory_path)

    def set_factory(self, factory_path):
        if self.__focused_context == None:
            return
        self.__panel.reset()
        factory = self.__factory_manager.get_factory(factory_path)
        self.__factory_manager.set_default_factory(factory)
        engine = factory.create_engine()
        self.__focused_context.set_engine(engine)
        self.__focused_context.set_enable(True)
        engine.focus_in()
        self.__panel.states_changed()

    def get_input_context_states(self, ic, conn):
        factory_path = ""
        context = self.__lookup_context(ic, conn)
        if context.get_factory() != None:
            factory_path = context.get_factory().get_object_path()
        return factory_path, context.is_enabled()

    def kill(self, conn):
        print "be killed"
        sys.exit(0)


class IBusProxy(ibus.IIBus):
    def __init__(self, bus, conn):
        super(IBusProxy, self).__init__(conn.get_dbusconn(), ibus.IBUS_PATH_IBUS)
        self.__ibus = bus
        self.__conn = conn
        self.__conn.connect("destroy", self.__conn_destroy_cb)

    def __conn_destroy_cb(self, conn):
        self.__conn = None
        self.__ibus = None

    def GetIBusAddress(self, dbusconn):
        return self.__ibus_addr

    def CreateInputContext(self, context_name, dbusconn):
        return self.__ibus.create_input_context(context_name, self.__conn)

    def ReleaseInputContext(self, ic, dbusconn):
        self.__ibus.release_input_context(ic, self.__conn)

    def RegisterFactories(self, object_paths, dbusconn):
        self.__ibus.register_factories(object_paths, self.__conn)

    def UnregisterEngines(self, object_paths, dbusconn):
        self.__ibus.unregister_engines(object_paths, self.__conn)

    def RegisterPanel(self, object_path, replace, dbusconn):
        self.__ibus.register_panel(object_path, replace, self.__conn)

    def RegisterConfig(self, object_path, replace, dbusconn):
        self.__ibus.register_config(object_path, replace, self.__conn)

    def ProcessKeyEvent(self, ic, keyval, is_press, state, \
                            dbusconn, reply_cb, error_cb):
        try:
            self.__ibus.process_key_event(ic, keyval, is_press, state,
                            self.__conn, reply_cb, error_cb)
        except Exception, e:
            error_cb(e)

    def GetFactories(self, dbusconn):
        return self.__ibus.get_factories()

    def GetFactoryInfo(self, factory_path, dbusconn):
        return self.__ibus.get_factory_info(factory_path)

    def SetFactory(self, factory_path, dbusconn):
        return self.__ibus.set_factory(factory_path)

    def GetInputContextStates(self, ic, dbusconn):
        return self.__ibus.get_input_context_states(ic, self.__conn)

    def RegisterListEngines(self, dbusconn):
        return self.__ibus.register_list_engines(self.__conn)

    def RegisterReloadEngines(self, dbusconn):
        return self.__ibus.register_reload_engines(self.__conn)

    def RegisterStartEngine(self, lang, name, dbusconn):
        return self.__ibus.register_start_engine(lang, name, self.__conn)

    def RegisterRestartEngine(self, lang, name, dbusconn):
        return self.__ibus.register_restart_engine(lang, name, self.__conn)

    def RegisterStopEngine(self, lang, name, dbusconn):
        return self.__ibus.register_stop_engine(lang, name, self.__conn)

    def Kill(self, dbusconn, reply_cb, error_cb):
        reply_cb()
        self.__ibus.kill(self.__conn)

