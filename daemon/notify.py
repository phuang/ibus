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

import dbus

try:
    __bus = dbus.SessionBus()
    __notify = __bus.get_object("org.freedesktop.Notifications", "/org/freedesktop/Notifications")
except:
    import traceback
    traceback.print_exc()
    __notify = None

__ignore_ids = set([])

def Notify(id, summary, body, icon):
    if id in __ignore_ids:
        return
    if __notify == None:
        return
    if icon == None:
        icon = "ibus"
    __notify.Notify("ibus", dbus.UInt32(id), icon, summary, body, \
        ["Do not show it again", "Do not show it again"], \
        dbus.Dictionary({"x" :100, "y" : 100}, signature="sv"), 5000)

if __name__ == "__main__":
    Notify(1, "A", "B", None)
