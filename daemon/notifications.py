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

import gobject
import ibus

class Notifications(ibus.Object):
    __gsignals__ = {
        "notification-closed" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_UINT, gobject.TYPE_UINT)),
        "action-invoked" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_UINT, gobject.TYPE_STRING)),
        }

    def __init__(self, ibusconn):
        super(Notifications, self).__init__()
        self.__ibusconn = ibusconn
        self.__notifications = self.__ibusconn.get_object(ibus.IBUS_NOTIFICATIONS_PATH)

        self.__ibusconn.connect("destroy", self.__ibusconn_destroy_cb)
        self.__ibusconn.connect("dbus-signal", self.__dbus_signal_cb)

    def notify(self, replaces_id, app_icon, summary, body, actions, expire_timeout):
        id = self.__notifications.Notify(
            replaces_id, app_icon, summary, body, actions, expire_timeout)
        return id

    def close_notification(self, id):
        return self.__notifications.CloseNotifications(id)

    def __dbus_signal_cb(self, ibusconn, message):
        if message.is_signal(ibus.IBUS_NOTIFICATIONS_IFACE, "NotificationsClosed"):
            args = message.get_args_list()
            self.emit("notifications-closed", args[0], args[1])
        elif message.is_signal(ibus.IBUS_NOTIFICATIONS_IFACE, "ActionInvoked"):
            args = message.get_args_list()
            self.emit("action-invoked", args[0], args[1])
        else:
            return False
        ibusconn.stop_emission("dbus-signal")
        return False

    def __ibusconn_destroy_cb(self, ibusconn):
        self.__ibusconn = None
        self.destroy()

gobject.type_register(Notifications)


class DummyNotifications(ibus.Object):
    __gsignals__ = {
        "notification-closed" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_UINT, gobject.TYPE_UINT)),
        "action-invoked" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_UINT, gobject.TYPE_STRING)),
        }

    def notify(self, replaces_id, app_icon, summary, body, actions, expire_timeout):
        return 0

    def close_notification(self, id):
        pass

gobject.type_register(DummyNotifications)


