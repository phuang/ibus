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
import dbus
import ibus
from ibus import keysyms
from ibus import modifier
from connection import Connection
from contextmanager import ContextManager
from factorymanager import FactoryManager
from panel import Panel, DummyPanel
from config import Config, DefaultConfig
from register import Register


class IBus(ibus.Object):
    def __init__(self):
        super(IBus, self).__init__()
        self.__context_manager = ContextManager()
        self.__factory_manager = FactoryManager()

        self.__panel = DummyPanel()
        self.__panel_handlers = list()
        self.__install_panel_handlers()

        self.__config = DefaultConfig()
        self.__config_handlers = list()
        self.__install_config_handlers()

        self.__register = Register()

        self.__config_watch = dict()

        self.__focused_context = None
        self.__last_focused_context = None
        self.__context_handlers = list()

        self.__connections = list()

        self.__prev_key = None

        self.__shortcut_trigger = self.__load_config_shortcut(
                ibus.CONFIG_GENERAL_SHORTCUT_TRIGGER,
                ibus.CONFIG_GENERAL_SHORTCUT_TRIGGER_DEFAULT)
        self.__shortcut_next_engine = self.__load_config_shortcut(
                ibus.CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE,
                ibus.CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE_DEFAULT)
        self.__shortcut_prev_engine = self.__load_config_shortcut(
                ibus.CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE,
                ibus.CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE_DEFAULT)

    def __load_config_shortcut(self, config_key, default_value):

        # load trigger
        shortcut_strings = default_value
        try:
            shortcut_strings = self.__config.get_value(config_key)
        except:
            pass
        shortcuts = []
        for s in shortcut_strings:
            keyval, keymask = self.__parse_shortcut_string(s)
            if keyval != 0:
                shortcuts.append((keyval, keymask))
        return shortcuts

    def new_connection(self, dbusconn):
        conn = Connection(dbusconn)
        IBusProxy(self, conn)
        conn.connect("destroy", self.__conn_destroy_cb)
        self.__connections.append(conn)

    def __conn_destroy_cb(self, conn):
        for key in conn.config_get_watch():
            self.config_remove_watch(key, conn)
        self.__connections.remove(conn)

    ##########################################################
    # methods for im context
    ##########################################################
    def create_input_context(self, name, conn):
        context = self.__context_manager.create_input_context(name, conn)
        factory = self.__factory_manager.get_default_factory()
        if factory:
            engine = factory.create_engine()
            context.set_engine(engine)
        return context.get_id()

    def release_input_context(self, ic, conn):
        self.__context_manager.release_input_context(ic, conn)

    def focus_in(self, ic, conn):
        context = self.__lookup_context(ic, conn)

        if self.__focused_context != context and self.__focused_context != None:
            self.__remove_focused_context_handlers()
            self.__focused_context.focus_out()

        self.__focused_context = context
        self.__install_focused_context_handlers()

        self.__panel.focus_in(context.get_id())
        self.__last_focused_context = context
        context.focus_in()
        self.__prev_key = None

    def focus_out(self, ic, conn):
        context = self.__lookup_context(ic, conn)

        if context == self.__focused_context:
            self.__remove_focused_context_handlers()
            self.__focused_context = None

        context.focus_out()
        self.__panel.focus_out(context.get_id())

    def reset(self, ic, conn):
        context = self.__lookup_context(ic, conn)
        context.reset()

    def is_enabled(self, ic, conn):
        context = self.__lookup_context(ic, conn)
        return context.is_enabled()

    def set_capabilities(self, ic, caps, conn):
        context = self.__lookup_context(ic, conn)
        return context.set_capabilities(caps)

    def process_key_event(self, ic, keyval, is_press, state,
                                conn, reply_cb, error_cb):
        context = self.__lookup_context(ic, conn)

        if self.__filter_keyboard_shortcuts(context, keyval, is_press, state):
            reply_cb(True)
            return
        else:
            context.process_key_event(keyval, is_press, state, reply_cb, error_cb)

    def set_cursor_location(self, ic, x, y, w, h, conn):
        context = self.__lookup_context(ic, conn)
        context.set_cursor_location(x, y, w, h)
        if context == self.__focused_context:
            self.__panel.set_cursor_location(x, y, w, h)

    def __context_enable(self, context):
        if context.get_engine() == None:
            factory = self.__factory_manager.get_default_factory()
            if factory:
                engine = factory.create_engine()
                engine.focus_in()
                context.set_engine(engine)
        context.set_enable(True)
        self.__panel.states_changed()

    def __context_disable(self, context):
        context.set_enable(False)
        self.__panel.reset()
        self.__panel.states_changed()

    def __context_next_factory(self, context):
        old_factory = context.get_factory()
        new_factory = self.__factory_manager.get_next_factory(old_factory)
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

    def __install_focused_context_handlers(self):
        # Install all callback functions
        if self.__focused_context == None:
            return
        signals = (
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
            id = self.__focused_context.connect(name, handler)
            self.__context_handlers.append(id)

    def __remove_focused_context_handlers(self):
        if self.__focused_context == None:
            return
        map(self.__focused_context.disconnect, self.__context_handlers)
        self.__context_handlers = []

    def __update_preedit_cb(self, context, text, attrs, cursor_pos, visible):
        assert self.__focused_context == context
        self.__panel.update_preedit(text, attrs, cursor_pos, visible)

    def __show_preedit_cb(self, context):
        assert self.__focused_context == context
        self.__panel.show_preedit()

    def __hide_preedit_cb(self, context):
        assert self.__focused_context == context
        self.__panel.hide_preedit()

    def __update_aux_string_cb(self, context, text, attrs, visible):
        assert self.__focused_context == context
        self.__panel.update_aux_string(text, attrs, visible)

    def __show_aux_string_cb(self, context):
        assert self.__focused_context == context
        self.__panel.show_aux_string()

    def __hide_aux_string_cb(self, context):
        assert self.__focused_context == context
        self.__panel.hide_aux_string()

    def __update_lookup_table_cb(self, context, lookup_table, visible):
        assert self.__focused_context == context
        self.__panel.update_lookup_table(lookup_table, visible)

    def __show_lookup_table_cb(self, context):
        assert self.__focused_context == context
        self.__panel.show_lookup_table()

    def __hide_lookup_table_cb(self, context):
        assert self.__focused_context == context
        self.__panel.hide_lookup_table()

    def __page_up_lookup_table_cb(self, context):
        assert self.__focused_context == context
        self.__panel.page_up_lookup_table()

    def __page_down_lookup_table_cb(self, context):
        assert self.__focused_context == context
        self.__panel.page_down_lookup_table()

    def __cursor_up_lookup_table_cb(self, context):
        assert self.__focused_context == context
        self.__panel.cursor_up_lookup_table()

    def __cursor_down_lookup_table_cb(self, context):
        assert self.__focused_context == context
        self.__panel.cursor_down_lookup_table()

    def __register_properties_cb(self, context, props):
        assert self.__focused_context == context
        self.__panel.register_properties(props)


    def __update_property_cb(self, context, prop):
        assert self.__focused_context == context
        self.__panel.update_property(prop)

    def __engine_lost_cb(self, context):
        assert self.__focused_context == context
        self.__panel.reset()

    def __context_destroy_cb(self, context):
        assert context == self.__focused_context
        self.__remove_focused_context_handlers()
        self.__panel.focus_out(context.get_id())
        self.__focused_context = None

    ##########################################################
    # methods for im engines
    ##########################################################
    def register_factories(self, object_paths, conn):
        self.__factory_manager.register_factories(object_paths, conn)


    ##########################################################
    # methods for panel
    ##########################################################
    def register_panel(self, object_path, replace, conn):
        if not isinstance(self.__panel, DummyPanel) and replace == False:
            raise ibus.Exception("has have a panel!")
        self.__uninstall_panel_handlers()
        self.__panel.destroy()

        self.__panel = Panel(conn, object_path)
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
            self.__panel = DummyPanel()

    ##########################################################
    # methods for panel
    ##########################################################
    def register_config(self, object_path, replace, conn):
        if not isinstance(self.__config, DefaultConfig) and replace == False:
            raise ibus.Exception("has have a config!")
        self.__uninstall_config_handlers()
        self.__config.destroy()
        self.__config = Config(conn, object_path)
        self.__install_config_handlers()
        for conn in self.__connections:
            conn.emit_dbus_signal("ConfigReload")

    def __install_config_handlers(self):
        signals = (
            ("value-changed", self.__config_value_changed_cb),
            ("destroy", self.__config_destroy_cb)
        )
        for signal, handler in signals:
            id = self.__config.connect(signal, handler)
            self.__config_handlers.append(id)

    def __uninstall_config_handlers(self):
        map(lambda id:self.__config.disconnect(id), self.__config_handlers)
        self.__config_handlers = list()

    def config_set_value(self, key, value, conn, **kargs):
        return self.__config.set_value(key, value, **kargs)

    def config_get_value(self, key, conn, **kargs):
        return self.__config.get_value(key, **kargs)

    def config_add_watch(self, key, conn):
        if not key.endswith("/"):
            key += "/"

        if conn.config_add_watch(key):
            if key not in self.__config_watch:
                self.__config_watch[key] = set()
            self.__config_watch[key].add(conn)

    def config_remove_watch(self, key, conn):
        if not key.endswith("/"):
            key += "/"

        if conn.config_remove_watch(key):
            if key in self.__config_watch:
                self.__config_watch[key].remove(conn)

    def __parse_shortcut_string(self, string):
        keys = string.split("+")
        keymask = 0
        for name, mask in modifier.MODIFIER_NAME_TABLE:
            if name in keys[:-1]:
                keymask |= mask
            keyname = keys[-1]
            if keyname[0] in "1234567890":
                keyname = "_" + keyname
            keyval = keysyms.__dict__.get(keyname, 0)

        return keyval, keymask

    def __config_value_changed_cb(self, config, key, value):
        for _dir in self.__config_watch.keys():
            if key.startswith(_dir):
                for conn in self.__config_watch[_dir]:
                    conn.emit_dbus_signal("ConfigValueChanged", key, value)

        # check daemon configure
        if key == ibus.CONFIG_GENERAL_SHORTCUT_TRIGGER:
            self.__shortcut_trigger = self.__load_config_shortcut(
                ibus.CONFIG_GENERAL_SHORTCUT_TRIGGER,
                ibus.CONFIG_GENERAL_SHORTCUT_TRIGGER_DEFAULT)
        elif key == ibus.CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE:
            self.__shortcut_next_engine = self.__load_config_shortcut(
                ibus.CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE,
                ibus.CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE_DEFAULT)
        elif key == ibus.CONFIG_GENERAL_SHORTCUT_PREV_ENGINE:
            self.__shortcut_prev_engine = self.__load_config_shortcut(
                ibus.CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE,
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
        super(IBusProxy, self).__init__(conn.get_dbusconn(), ibus.IBUS_PATH)
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

    def SetCursorLocation(self, ic, x, y, w, h, dbusconn):
        self.__ibus.set_cursor_location(ic, x, y, w, h, self.__conn)

    def FocusIn(self, ic, dbusconn):
        self.__ibus.focus_in(ic, self.__conn)

    def FocusOut(self, ic, dbusconn):
        self.__ibus.focus_out(ic, self.__conn)

    def Reset(self, ic, dbusconn):
        self.__ibus.reset(ic, self.__conn)

    def IsEnabled(self, ic, dbusconn):
        return self.__ibus.is_enabled(ic, self.__conn)

    def SetCapabilities(self, ic, caps, dbusconn):
        return self.__ibus.set_capabilities(ic, caps, self.__conn)

    def GetFactories(self, dbusconn):
        return self.__ibus.get_factories()

    def GetFactoryInfo(self, factory_path, dbusconn):
        return self.__ibus.get_factory_info(factory_path)

    def SetFactory(self, factory_path, dbusconn):
        return self.__ibus.set_factory(factory_path)

    def GetInputContextStates(self, ic, dbusconn):
        return self.__ibus.get_input_context_states(ic, self.__conn)

    def ConfigAddWatch(self, key, dbusconn):
        return self.__ibus.config_add_watch(key, self.__conn)

    def ConfigRemoveWatch(self, key, dbusconn):
        return self.__ibus.config_remove_watch(key, self.__conn)

    def ConfigSetValue(self, key, value, dbusconn, reply_cb, error_cb):
        self.__ibus.config_set_value(key, value, self.__conn,
                reply_handler = reply_cb,
                error_handler = error_cb)

    def ConfigGetValue(self, key, dbusconn, reply_cb, error_cb):
        self.__ibus.config_get_value(key, self.__conn,
                reply_handler = reply_cb,
                error_handler = error_cb)

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

