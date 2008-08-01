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

import gobject
import dbus.lowlevel
import dbus.connection
import dbus.mainloop.glib
import ibus

dbus.mainloop.glib.DBusGMainLoop(set_as_default = True)

class Bus(ibus.Object):
    __gsignals__ = {
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
        self.__bus = self.__dbusconn.get_object(ibus.IBUS_NAME, ibus.IBUS_PATH)
        self.__dbusconn.add_message_filter(self.__dbus_message_cb)

    def __dbus_message_cb(self, conn, message):
        if message.is_signal(ibus.IBUS_IFACE, "ConfigValueChanged"):
            args = message.get_args_list()
            key, value = args[0], args[1]
            self.emit("config-value-changed", key, value)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(ibus.IBUS_IFACE, "ConfigReloaded"):
            self.emit("config-reloaded", key, value)
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        elif message.is_signal(dbus.LOCAL_IFACE, "Disconnected"):
            self.destroy()
            retval = dbus.lowlevel.HANDLER_RESULT_HANDLED
        else:
            retval = dbus.lowlevel.HANDLER_RESULT_NOT_YET_HANDLED

        return retval

    def get_dbusconn(self):
        return self.__dbusconn

    def get_address(self):
        return ibus.IBUS_ADDR

    def create_input_context(self, client_name):
        return self.__bus.CreateInputContext(client_name)

    def release_input_context(self, ic):
        return self.__bus.ReleaseInputContext(ic)

    def process_key_event(self, ic, keyval, is_press, state):
        return self.__bus.ProcessKeyEvent(ic, keyval, is_press, state)

    def set_cursor_location(self, ic, x, y, w, h):
        return self.__bus.SetCursorLocation(ic, x, y, w, h)

    def foucs_in(self, ic):
        return self.__bus.FocusIn(ic)

    def foucs_out(self, ic):
        return self.__bus.FocusOut(ic)

    def reset(self, ic):
        return self.__bus.Reset(ic)

    def is_enabled(self, ic):
        return self.__bus.IsEnabled(ic)

    def set_capabilities(self, ic, caps):
        return self.__bus.set_capabilities(ic, caps)

    def register_factories(self, object_paths):
        return self.__bus.RegisterFactories(object_paths, **ibus.DEFAULT_ASYNC_HANDLERS)

    def unregister_factories(self, object_paths):
        return self.__bus.UnregisterFactories(object_paths)

    def register_panel(self, object_path, replace = False):
        return self.__bus.RegisterPanel(object_path, replace)

    def register_config(self, object_path, replace = False):
        return self.__bus.RegisterConfig(object_path, replace)

    def get_factories(self):
        return self.__bus.GetFactories()

    def get_factory_info(self, factory_path):
        return self.__bus.GetFactoryInfo(factory_path)

    def set_factory(self, factory_path):
        return self.__bus.SetFactory(factory_path)

    def get_input_context_states(self, ic):
        return self.__bus.GetInputContextStates(ic)

    def config_add_watch(self, key):
        return self.__bus.ConfigAddWatch(key)

    def config_remove_watch(self, key):
        return self.__bus.ConfigRemoveWatch(key)

    def config_set_value(self, key, value):
        return self.__bus.ConfigSetValue(key, value)

    def config_set_list(self, key, value, list_type):
        value = dbus.Array(value, signature = list_type)
        return self.__bus.ConfigSetValue(key, value)

    def config_get_value(self, key, default_value = None):
        try:
            return self.__bus.ConfigGetValue(key)
        except Exception, e:
            return default_value

    def register_list_engines(self):
        return self.__bus.RegisterListEngines()

    def register_start_engine(self, lang, name):
        return self.__bus.RegisterStartEngine(lang, name)

    def register_restart_engine(self, lang, name):
        return self.__bus.RegisterRestartEngine(lang, name)

    def register_stop_engine(self, lang, name):
        return self.__bus.RegisterStopEngine(lang, name)

