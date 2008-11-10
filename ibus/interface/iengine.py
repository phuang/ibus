# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright(c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or(at your option) any later version.
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

__all__ = ("IEngine", )

import dbus.service
from ibus.common import \
    IBUS_IFACE_ENGINE

class IEngine(dbus.service.Object):
    # define method decorator.
    method = lambda **args: \
        dbus.service.method(dbus_interface=IBUS_IFACE_ENGINE, \
                            **args)

    # define signal decorator.
    signal = lambda **args: \
        dbus.service.signal(dbus_interface=IBUS_IFACE_ENGINE, \
                            **args)

    # define async method decorator.
    async_method = lambda **args: \
        dbus.service.method(dbus_interface=IBUS_IFACE_ENGINE, \
                            async_callbacks=("reply_cb", "error_cb"), \
                            **args)

    @method(in_signature="ubu", out_signature="b")
    def ProcessKeyEvent(self, keyval, is_press, state):
        pass

    @method(in_signature="iiii")
    def SetCursorLocation(self, x, y, w, h): pass

    @method()
    def FocusIn(self): pass

    @method()
    def FocusOut(self): pass

    @method()
    def Reset(self): pass

    # signals for lookup table
    @method()
    def PageUp(self): pass

    @method()
    def PageDown(self): pass

    @method()
    def CursorUp(self): pass

    @method()
    def CursorDown(self): pass

    @method()
    def Enable(self): pass

    @method()
    def Disable(self): pass

    @method(in_signature="si")
    def PropertyActivate(self, prop_name, prop_state): pass

    @method(in_signature="s")
    def PropertyShow(self, prop_name): pass

    @method(in_signature="s")
    def PropertyHide(self, prop_name): pass

    @method()
    def Destroy(self): pass

    @signal(signature="s")
    def CommitString(self, text): pass

    @signal(signature="ubu")
    def ForwardKeyEvent(self, keyval, is_press, state): pass

    @signal(signature="sa(uuuu)ib")
    def UpdatePreedit(self, text, attrs, cursor_pos, visible): pass

    @signal()
    def ShowPreedit(self): pass

    @signal()
    def HidePreedit(self): pass

    @signal(signature="sa(uuuu)b")
    def UpdateAuxString(self, text, attrs, visible): pass

    @signal()
    def ShowAuxString(self): pass

    @signal()
    def HideAuxString(self): pass

    @signal(signature="vb")
    def UpdateLookupTable(self, lookup_table, visible): pass

    @signal()
    def ShowLookupTable(self): pass

    @signal()
    def HideLookupTable(self): pass

    @signal()
    def PageUpLookupTable(self): pass

    @signal()
    def PageDownLookupTable(self): pass

    @signal()
    def CursorUpLookupTable(self): pass

    @signal()
    def CursorDownLookupTable(self): pass

    @signal(signature="a(susssbbuv)")
    def RegisterProperties(self, props): pass

    @signal(signature="v")
    def UpdateProperty(self, prop): pass

