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
        "IBUS_ADDR",
        "IBUS_IFACE",
        "IBUS_NAME",
        "IBUS_PATH",
        "IBUS_CONFIG_IFACE",
        "IBUS_ENGINE_FACTORY_IFACE",
        "IBUS_ENGINE_IFACE",
        "IBUS_PANEL_IFACE",
        "default_reply_handler",
        "default_error_handler",
        "DEFAULT_ASYNC_HANDLERS"
    )

import os
import sys

display = os.environ["DISPLAY"]
if "." not in display:
    display += ".0" 

__username = None
try:
    __username = os.getlogin()
except:
    pass
if not __username:
    __username = os.getenv ("LOGNAME")
if not __username:
    __username = os.getenv ("USER")
if not __username:
    __username = os.getenv ("LNAME")
if not __username:
    __username = os.getenv ("USERNAME")

IBUS_ADDR = "unix:path=/tmp/ibus-%s/ibus-%s" % (__username, display.replace(":", "-"))
# IBUS_ADDR  = "tcp:host=localhost,port=7799"

IBUS_IFACE = "org.freedesktop.IBus"
IBUS_PATH  = "/org/freedesktop/IBus"
IBUS_NAME  = "org.freedesktop.IBus"

IBUS_CONFIG_IFACE = "org.freedesktop.IBus.Config"
IBUS_ENGINE_FACTORY_IFACE = "org.freedesktop.IBus.EngineFactory"
IBUS_ENGINE_IFACE = "org.freedesktop.IBus.Engine"
IBUS_PANEL_IFACE = "org.freedesktop.IBus.Panel"

def default_reply_handler( *args):
    pass

def default_error_handler(e):
    print >> sys.stderr, e

DEFAULT_ASYNC_HANDLERS = {
    "reply_handler" : default_reply_handler,
    "error_handler" : default_error_handler
}
