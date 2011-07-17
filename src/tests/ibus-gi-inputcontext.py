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
IBus.init()
main = glib.MainLoop()
bus = IBus.Bus()
ic = bus.create_input_context("ibus-test")
ic.get_engine()
ic.get_engine()
ic.get_engine()
ic.get_engine()
