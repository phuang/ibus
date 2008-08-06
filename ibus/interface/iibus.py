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

__all__ = ("IIBus", )

import dbus.service
from ibus.common import \
    IBUS_IFACE

class IIBus(dbus.service.Object):
    # define method decorator.
    method = lambda **args: \
        dbus.service.method(dbus_interface = IBUS_IFACE, \
                            connection_keyword = "dbusconn", \
                            **args)

    # define async method decorator.
    async_method = lambda **args: \
        dbus.service.method(dbus_interface = IBUS_IFACE, \
                            connection_keyword = "dbusconn", \
                            async_callbacks = ("reply_cb", "error_cb"), \
                            **args)

    @method(out_signature = "s")
    def GetIBusAddress(self, dbusconn): pass

    # methods for ibus clients
    @method(in_signature = "s", out_signature = "s")
    def CreateInputContext(self, client_name, dbusconn): pass

    @method(in_signature = "s")
    def ReleaseInputContext(self, ic, dbusconn): pass

    @async_method(in_signature = "subu", out_signature = "b")
    def ProcessKeyEvent(self, ic, keyval, is_press, state, dbusconn, reply_cb, error_cb): pass

    @method(in_signature = "siiii")
    def SetCursorLocation(self, ic, x, y, w, h, dbusconn): pass

    @method(in_signature = "s")
    def FocusIn(self, ic, dbusconn): pass

    @method(in_signature = "s")
    def FocusOut(self, ic, dbusconn): pass

    @method(in_signature = "s")
    def Reset(self, ic, dbusconn): pass

    @method(in_signature = "s", out_signature = "b")
    def IsEnabled(self, ic, dbusconn): pass

    @method(in_signature = "si")
    def SetCapabilities(self, ic, caps, dbusconn): pass

    # methods for ibus engine provide
    @method(in_signature = "ao")
    def RegisterFactories(self, object_paths, dbusconn): pass

    @method(in_signature = "ao")
    def UnregisterFactories(self, object_paths, dbusconn): pass

    # methods for ibus panel
    @method(in_signature = "ob")
    def RegisterPanel(self, object_path, replace, dbusconn): pass

    # methods for ibus config
    @method(in_signature = "ob")
    def RegisterConfig(self, object_path, replace, dbusconn): pass

    # general methods
    @method(out_signature = "av")
    def GetFactories(self, dbusconn): pass

    @method(in_signature = "o", out_signature = "av")
    def GetFactoryInfo(self, factory_path, dbusconn): pass

    @method(in_signature = "o")
    def SetFactory(self, factory_path, dbusconn): pass

    @method(in_signature = "s", out_signature = "sb")
    def GetInputContextStates(self, ic, dbusconn): pass

    @method(in_signature = "s")
    def ConfigAddWatch(self, key, dbusconn): pass

    @method(in_signature = "s")
    def ConfigRemoveWatch(self, key, dbusconn): pass

    @async_method(in_signature = "sv")
    def ConfigSetValue(self, key, value, dbusconn, reply_cb, error_cb): pass

    @async_method(in_signature = "s", out_signature = "v")
    def ConfigGetValue(self, key, dbusconn, reply_cb, error_cb): pass

    @method(out_signature = "a(ssssssb)")
    def RegisterListEngines(self, dbusconn): pass

    @method(in_signature = "ss")
    def RegisterStartEngine(self, lang, name, dbusconn): pass

    @method(in_signature = "ss")
    def RegisterRestartEngine(self, lang, name, dbusconn): pass

    @method(in_signature = "ss")
    def RegisterStopEngine(self, lang, name, dbusconn): pass

    @async_method()
    def Kill(self, dbusconn, reply_cb, error_cb): pass

    #sigals
    def CommitString(self, ic, text): pass

    def UpdatePreedit(self, ic, text, attrs, cursor_pos, show): pass

    def Enabled(self, ic): pass

    def Disabled(self, ic): pass

    def ConfigValueChanged(self, key, value): pass

    def ConfigReload(self): pass
