#!/usr/bin/env python
# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2011 Peng Huang <shawn.p.huang@gmail.com>
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


import glib
import gio
from gi.repository import IBus

main = glib.MainLoop()

bus = IBus.Bus()
count = 2

def callback(bus, res, main):
    global count
    try:
        results = bus.is_global_engine_enabled_async_finish(res)
        print "Success:", results
    except Exception as e:
        print "Failure:", e

    count -= 1
    if count == 0:
        main.quit()

bus.is_global_engine_enabled_async(-1, None, callback, main)

cancellable = gio.Cancellable()
bus.is_global_engine_enabled_async(-1, cancellable, callback, main)
cancellable.cancel()

main.run()
