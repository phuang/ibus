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

import gtk
import gtk.gdk as gdk
import gobject
import ibus
import dbus
import icon as _icon
import os
import sys
from os import path
from ibus import interface

from gettext import dgettext
_  = lambda a : dgettext("ibus", a)
N_ = lambda a : a

class Notifications(ibus.NotificationsBase):
    def __init__ (self, bus):
        super(Notifications, self).__init__(bus)
        self.__bus = bus
        self.__dbus = dbus.SessionBus()
        self.__notifications = self.__dbus.get_object(
                "org.freedesktop.Notifications", "/org/freedesktop/Notifications")
    def notify(self, replaces_id, app_icon, summary, body, actions, expire_timeout):
        hints = dbus.Dictionary(signature="sv")
        hints["desktop-entry"] = "ibus"
        hints["x"] = 0
        hints["y"] = 0
        return self.__notifications.Notify("ibus",
                        replaces_id,
                        app_icon,
                        summary,
                        body,
                        actions,
                        hints,
                        expire_timeout)

    def close_notification(self, id):
        return self.__notifications.CloseNotifications(id)

    def notification_closed(self, id, reason):
        self.__proxy.NotificationClosed(id, reason)

    def action_invoked(self, id, action_key):
        self.__proxy.ActionInvoked(id, action_key)
