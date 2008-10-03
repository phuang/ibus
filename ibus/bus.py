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

__all__ = (
        "Bus",
    )

import sys
import gobject
import dbus
import dbus.lowlevel
import dbus.connection
import dbus.mainloop.glib
import ibus

dbus.mainloop.glib.DBusGMainLoop(set_as_default = True)

class Bus(ibus.Object):
    __gsignals__ = {
        "commit-string" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_STRING)
        ),
        "update-preedit" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_STRING, gobject.TYPE_PYOBJECT, gobject.TYPE_INT, gobject.TYPE_BOOLEAN)
        ),
        "show-preedit" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, )
        ),
        "hide-preedit" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, )
        ),
        "update-aux-string" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_STRING, gobject.TYPE_PYOBJECT, gobject.TYPE_BOOLEAN)
        ),
        "show-aux-string" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, )
        ),
        "hide-aux-string" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, )
        ),
        "update-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_PYOBJECT, gobject.TYPE_BOOLEAN)
        ),
        "show-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, )
        ),
        "hide-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, )
        ),
        "page-up-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, )
        ),
        "page-down-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, )
        ),
        "cursor-up-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, )
        ),
        "cursor-down-lookup-table" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, )
        ),
        "config-value-changed" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_PYOBJECT)
        ),
        "config-reloaded" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            ()
        ),
    }

    def __init__(self):
        super(Bus, self).__init__()
        self.__dbusconn = dbus.connection.Connection(ibus.IBUS_ADDR)
        self.__ibus = self.__dbusconn.get_object(ibus.IBUS_NAME, ibus.IBUS_PATH)
        self.__dbus = self.__dbusconn.get_object(dbus.BUS_DAEMON_NAME, dbus.BUS_DAEMON_PATH)
        try:
            unique_name = self.get_name_owner(ibus.IBUS_CONFIG_NAME)
            self.__config = self.__dbusconn.get_object(unique_name, ibus.IBUS_CONFIG_PATH)
        except:
            self.__config = None
        self.__dbusconn.add_message_filter(self.__dbus_message_cb)
        self.add_match(
            "type='signal',"
            "interface='" + dbus.BUS_DAEMON_IFACE + "',"
            "member='NameOwnerChanged',"
            "arg0='" + ibus.IBUS_CONFIG_NAME + "'")

    # define dbus methods
    def get_dbus(self):
        return self.__dbus

    def hello(self):
        return self.__dbus.Hello()

    def request_name(self, name, flags):
        return self.__dbus.RequestName(name, flags)

    def release_name(self, name):
        return self.__dbus.ReleaseName(name)

    def get_name_owner(self, name):
        return self.__dbus.GetNameOwner(name)

    def add_match(self, rule):
        return self.__dbus.AddMatch(rule)

    def remove_match(self, rule):
        return self.__dbus.RemoveMatch(rule)

    def get_dbusconn(self):
        return self.__dbusconn

    def get_address(self):
        return ibus.IBUS_ADDR

    def create_input_context(self, client_name):
        return self.__ibus.CreateInputContext(client_name)

    def release_input_context(self, ic):
        return self.__ibus.ReleaseInputContext(ic)

    def process_key_event(self, ic, keyval, is_press, state):
        return self.__ibus.ProcessKeyEvent(ic, keyval, is_press, state)

    def set_cursor_location(self, ic, x, y, w, h):
        return self.__ibus.SetCursorLocation(ic, x, y, w, h)

    def foucs_in(self, ic):
        return self.__ibus.FocusIn(ic)

    def foucs_out(self, ic):
        return self.__ibus.FocusOut(ic)

    def reset(self, ic):
        return self.__ibus.Reset(ic)

    def is_enabled(self, ic):
        return self.__ibus.IsEnabled(ic)

    def set_capabilities(self, ic, caps):
        return self.__ibus.SetCapabilities(ic, caps)

    def register_factories(self, object_paths):
        return self.__ibus.RegisterFactories(object_paths, **ibus.DEFAULT_ASYNC_HANDLERS)

    def unregister_factories(self, object_paths):
        return self.__ibus.UnregisterFactories(object_paths)

    def register_config(self, object_path, replace = False):
        return self.__ibus.RegisterConfig(object_path, replace)

    def get_factories(self):
        return self.__ibus.GetFactories()

    def get_factory_info(self, factory_path):
        return self.__ibus.GetFactoryInfo(factory_path)

    def set_factory(self, factory_path):
        return self.__ibus.SetFactory(factory_path)

    def get_input_context_states(self, ic):
        return self.__ibus.GetInputContextStates(ic)

    def config_add_watch(self, section):
        return self.add_match(
                    "type='signal',"
                    "interface='" + ibus.IBUS_CONFIG_NAME + "',"
                    "member='ValueChanged',"
                    "arg0='" + section + "'"
                    )

    def config_remove_watch(self, section):
        return self.remove_match(
                    "type='signal',"
                    "interface='" + ibus.IBUS_CONFIG_NAME + "',"
                    "member='ValueChanged',"
                    "arg0='" + section + "'"
                    )

    def config_set_value(self, section, name, value):
        return self.__config.SetValue(section, name, value)

    def config_set_list(self, section, name, value, list_type):
        value = dbus.Array(value, signature = list_type)
        return self.__config.SetValue(section, name, value)

    def config_get_value(self, section, name, default_value = None):
        try:
            return self.__config.GetValue(section, name)
        except Exception, e:
            return default_value

    def register_list_engines(self):
        return self.__ibus.RegisterListEngines()

    def register_reload_engines(self):
        return self.__ibus.RegisterReloadEngines()

    def register_start_engine(self, lang, name):
        return self.__ibus.RegisterStartEngine(lang, name)

    def register_restart_engine(self, lang, name):
        return self.__ibus.RegisterRestartEngine(lang, name)

    def register_stop_engine(self, lang, name):
        return self.__ibus.RegisterStopEngine(lang, name)

    def kill(self):
        return self.__ibus.Kill()

    def __dbus_message_cb(self, conn, message):
        # name owner changed signal
        if message.is_signal(dbus.BUS_DAEMON_IFACE, "NameOwnerChanged"):
            args = message.get_args_list()
            if args[0] == ibus.IBUS_CONFIG_NAME:
                if args[2] != "":
                    self.__config = self.__dbusconn.get_object(args[2], ibus.IBUS_CONFIG_PATH)
                else:
                    self.__config = None
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        # commit string signal
        elif message.is_signal(ibus.IBUS_IFACE, "CommitString"):
            args = message.get_args_list()
            ic, string = args[0:2]
            self.emit("commit-string", ic, string.encode("utf-8"))
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED

        # preedit signals
        elif message.is_signal(ibus.IBUS_IFACE, "UpdatePreedit"):
            args = message.get_args_list()
            ic, preedit, attrs, cursor_pos, visible = args[0:5]
            attrs = ibus.attr_list_from_dbus_value(attrs)
            self.emit("update-preedit", ic, preedit.encode("utf-8"),
                        attrs, cursor_pos, visible)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(ibus.IBUS_IFACE, "ShowPreedit"):
            args = message.get_args_list()
            ic = args[0]
            self.emit("show-preedit", ic)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(ibus.IBUS_IFACE, "HidePreedit"):
            args = message.get_args_list()
            ic = args[0]
            self.emit("hide-preedit", ic)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED

        # aux string signals
        elif message.is_signal(ibus.IBUS_IFACE, "UpdateAuxString"):
            args = message.get_args_list()
            ic, aux_string, attrs, visible = args[0:4]
            attrs = ibus.attr_list_from_dbus_value(attrs)
            self.emit("update-aux-string", ic, aux_string.encode("utf-8"),
                        attrs, visible)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(ibus.IBUS_IFACE, "ShowAuxString"):
            args = message.get_args_list()
            ic = args[0]
            self.emit("show-aux-string", ic)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(ibus.IBUS_IFACE, "HideAuxString"):
            args = message.get_args_list()
            ic = args[0]
            self.emit("hide-aux-string", ic)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED

        # lookup table signals
        elif message.is_signal(ibus.IBUS_IFACE, "UpdateLookupTable"):
            args = message.get_args_list()
            ic, lookup_table, visible = args[0:3]
            lookup_table = ibus.lookup_table_from_dbus_value(lookup_table)
            self.emit("update-lookup-table", ic, lookup_table, visible)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(ibus.IBUS_IFACE, "ShowLookupTable"):
            args = message.get_args_list()
            ic = args[0]
            self.emit("show-lookup-table", ic)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(ibus.IBUS_IFACE, "HideLookupTable"):
            args = message.get_args_list()
            ic = args[0]
            self.emit("hide-lookup-table", ic)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(ibus.IBUS_IFACE, "PageUpLookupTable"):
            args = message.get_args_list()
            ic = args[0]
            self.emit("page-up-lookup-table", ic)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(ibus.IBUS_IFACE, "PageDownLookupTable"):
            args = message.get_args_list()
            ic = args[0]
            self.emit("page-down-lookup-table", ic)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(ibus.IBUS_IFACE, "CursorUpLookupTable"):
            args = message.get_args_list()
            ic = args[0]
            self.emit("cursor-up-lookup-table", ic)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(ibus.IBUS_IFACE, "CursorDownLookupTable"):
            args = message.get_args_list()
            ic = args[0]
            self.emit("cursor-down-lookup-table", ic)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED

        # Config signals
        elif message.is_signal(ibus.IBUS_IFACE, "ConfigValueChanged"):
            args = message.get_args_list()
            key, value = args[0:2]
            self.emit("config-value-changed", key, value)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(ibus.IBUS_IFACE, "ConfigReloaded"):
            self.emit("config-reloaded")
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED

        # DBUS Disconnected signal
        elif message.is_signal(dbus.LOCAL_IFACE, "Disconnected"):
            self.destroy()
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        else:
            retval = dbus.lowlevel.HANDLER_RESULT_NOT_YET_HANDLED
        return retval
