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
        "IBus",
    )
import dbus.connection
import ibus
import dbus.mainloop.glib

dbus.mainloop.glib.DBusGMainLoop(set_as_default = True)

class IBus(dbus.connection.Connection):
    def __new__(cls):
        self = super(IBus, cls).__new__(cls, ibus.IBUS_ADDR)
        self.__ibus = self.get_object(ibus.IBUS_NAME, ibus.IBUS_PATH)
        return self

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
        return self.__ibus.set_capabilities(ic, caps)

    def register_factories(self, object_paths):
        return self.__ibus.RegisterFactories(object_paths, **ibus.DEFAULT_ASYNC_HANDLERS)

    def unregister_factories(self, object_paths):
        return self.__ibus.UnregisterFactories(object_paths)

    def register_panel(self, object_path, replace = False):
        return self.__ibus.RegisterPanel(object_path, replace)

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

    def config_set_value(self, key, value):
        return self.__ibus.ConfigSetValue(key, value)

    def config_get_value(self, key):
        return self.__ibus.ConfigGetValue(key)

    def register_list_engines(self):
        return self.__ibus.RegisterListEngines()

    def register_start_engine(self, lang, name):
        return self.__ibus.RegisterStartEngine(lang, name)

    def register_restart_engine(self, lang, name):
        return self.__ibus.RegisterRestartEngine(lang, name)

    def register_stop_engine(self, lang, name):
        return self.__ibus.RegisterStopEngine(lang, name)

