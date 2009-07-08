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
    IBUS_IFACE_IBUS

class IIBus(dbus.service.Object):
    # define method decorator.
    method = lambda **args: \
        dbus.service.method(dbus_interface=IBUS_IFACE_IBUS, \
                            connection_keyword="dbusconn", \
                            **args)

    # define async method decorator.
    async_method = lambda **args: \
        dbus.service.method(dbus_interface=IBUS_IFACE_IBUS, \
                            connection_keyword="dbusconn", \
                            async_callbacks=("reply_cb", "error_cb"), \
                            **args)

    @method(out_signature="s")
    def GetAddress(self, dbusconn): pass

    # methods for ibus clients
    @method(in_signature="s", out_signature="o")
    def CreateInputContext(self, client_name, dbusconn): pass

    @method(in_signature="s", out_signature="sb")
    def GetInputContextStates(self, ic, dbusconn): pass

    # methods for ibus engine provide
    @method(in_signature="av")
    def RegisterFactories(self, object_paths, dbusconn): pass

    @method(in_signature="av")
    def UnregisterFactories(self, object_paths, dbusconn): pass

    # general methods
    @method(in_signature="av")
    def RegisterComponent(self, components, dbusconn): pass

    @method(out_signature="av")
    def ListEngines(self, dbusconn): pass

    @method(out_signature="av")
    def ListActiveEngines(self, dbusconn): pass

    @method(in_signature="b")
    def Exit(self, restart, dbusconn): pass

    @method(in_signature="v", out_signature="v")
    def Ping(self, data, dbusconn): pass

