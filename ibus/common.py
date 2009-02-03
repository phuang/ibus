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
        "IBUS_NOTIFICATIONS_IFACE",
        "default_reply_handler",
        "default_error_handler",
        "DEFAULT_ASYNC_HANDLERS",
        "CONFIG_GENERAL_SHORTCUT_TRIGGER",
        "CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE",
        "CONFIG_GENERAL_SHORTCUT_PREV_ENGINE",
        "CONFIG_GENERAL_SHORTCUT_TRIGGER_DEFAULT",
        "CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE_DEFAULT",
        "CONFIG_GENERAL_SHORTCUT_PREV_ENGINE_DEFAULT",
        "main",
        "main_quit",
        "main_iteration"
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

IBUS_CONFIG_IFACE = "org.freedesktop.ibus.Config"
IBUS_ENGINE_FACTORY_IFACE = "org.freedesktop.ibus.EngineFactory"
IBUS_ENGINE_IFACE = "org.freedesktop.ibus.Engine"
IBUS_PANEL_IFACE = "org.freedesktop.ibus.Panel"
IBUS_NOTIFICATIONS_IFACE = "org.freedesktop.ibus.Notifications"

def default_reply_handler( *args):
    pass

def default_error_handler(e):
    print >> sys.stderr, e

DEFAULT_ASYNC_HANDLERS = {
    "reply_handler" : default_reply_handler,
    "error_handler" : default_error_handler
}

CONFIG_GENERAL_SHORTCUT_TRIGGER     = "/general/keyboard_shortcut_trigger"
CONFIG_GENERAL_SHORTCUT_TRIGGER_DEFAULT = [
    "Ctrl+space",
    "Zenkaku_Hankaku",
    "Hangul"]
CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE = "/general/keyboard_shortcut_next_engine"
CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE_DEFAULT = []
CONFIG_GENERAL_SHORTCUT_PREV_ENGINE = "/general/keyboard_shortcut_prev_engine"
CONFIG_GENERAL_SHORTCUT_PREV_ENGINE_DEFAULT = []

__mainloop = None

def __init_main_loop():
    global __mainloop
    if __mainloop == None:
        import gobject
        __mainloop = gobject.MainLoop()

def main():
    __init_main_loop()
    __mainloop.run()

def main_quit():
    global __mainloop
    if __mainloop:
        __mainloop.quit()
    
def main_iteration(may_block=False):
    __init_main_loop()
    return __mainloop.get_context().iteration(may_block)
