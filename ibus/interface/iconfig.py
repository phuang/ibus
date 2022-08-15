# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2007-2010 Red Hat, Inc.
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

__all__ = ("IConfig", )

import dbus.service
from ibus.common import \
    IBUS_IFACE_CONFIG

class IConfig(dbus.service.Object):
    # define method decorator.
    method = lambda **args: \
        dbus.service.method(dbus_interface=IBUS_IFACE_CONFIG, \
            **args)

    # define signal decorator.
    signal = lambda **args: \
        dbus.service.signal(dbus_interface=IBUS_IFACE_CONFIG, \
            **args)

    # define async method decorator.
    async_method = lambda **args: \
        dbus.service.method(dbus_interface=IBUS_IFACE_CONFIG, \
            async_callbacks=("reply_cb", "error_cb"), \
            **args)

    @method(in_signature="ss", out_signature="v")
    def GetValue(self, section, name): pass

    @method(in_signature="s", out_signature="s{sv}")
    def GetValues(self, section): pass

    @method(in_signature="ssv")
    def SetValue(self, section, name, value): pass

    @method(in_signature="ss")
    def UnsetValue(self, section, name): pass

    @method()
    def Destroy(self): pass

    @signal(signature="ssv")
    def ValueChanged(self, section, name, value): pass

