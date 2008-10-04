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
import dbus
import dbus.service
import ibus

class MatchRule:
    def __init__(self, rule):
        m = self.__eval_rule(rule)
        self.__type = m.get('type', None)
        self.__sender = m.get('sender', None)
        self.__interface = m.get('interface', None)
        self.__member = m.get('member', None)
        self.__path = m.get('path', None)
        self.__destination = m.get('destination', None)
        self.__args = []
        for i in range(64):
            arg = m.get('arg%d'%i, None)
            if arg == None:
                continue
            self.__args.append((i, arg))

    def __eval_rule(self, rule):
        _fn = lambda **kargs: kargs
        try:
            return eval("_fn(%s)" % rule)
        except:
            raise ibus.IBusException("Parse match rule failed\n%s" % rule)

    def __cmp_bus_name(self, dbusobj, a, b):
        if a == None or b == None:
            return a == b
        a_is_unique = a.startswith(":")
        b_is_unique = b.startswith(":")
        if a_is_unique == b_is_unique:
            return a == b
        else:
            try:
                if not a_is_unique:
                    a = dbusobj.get_name_owner(a).get_unique_name()
                if not b_is_unique:
                    b = dbusobj.get_name_owner(b).get_unique_name()
                return a == b
            except:
                return False

    def match_message(self, dbusobj, message):
        if self.__type == 'signal' and message.get_type() != 4:
            return False

        if self.__sender:
            if not self.__cmp_bus_name(dbusobj, self.__sender, message.get_sender()):
                return False

        if self.__interface and self.__interface != message.get_interface():
            return False

        if self.__member and self.__member != message.get_member():
            return False

        if self.__path and self.__path != message.get_path():
            return False

        if self.__destination:
            if not self.__cmp_bus_name(dbusobj, self.__destination, message.get_destination()):
                return False
        args = message.get_args_list()
        for i, arg in self.__args:
            if i >= len(args):
                return False
            if arg != args[i]:
                return False
        return True

class DBusReal(ibus.Object):
    __gsignals__ = {
        "name-owner-changed" : (
            gobject.SIGNAL_RUN_FIRST,
            gobject.TYPE_NONE,
            (gobject.TYPE_STRING, gobject.TYPE_STRING, gobject.TYPE_STRING)),
    }

    def __init__(self):
        super(DBusReal, self).__init__()
        self.__connections = dict()
        self.__unique_name_dict = dict()
        self.__name_dict = dict()
        self.__id = 0

    def add_connection(self, ibusconn):
        self.__connections[ibusconn] = list()
        ibusconn.connect("dbus-message", self.__dbus_message_cb)
        ibusconn.connect("destroy", self.__ibusconn_destroy_cb)

    def register_connection(self, ibusconn):
        name = ibusconn.get_unique_name()
        if name:
            raise ibus.IBusException("Already handled an Hello message")
        self.__id += 1
        name = ":1.%d" % self.__id
        self.__unique_name_dict[name] = ibusconn
        ibusconn.set_unique_name(name)
        return name

    def list_names(self):
        return self.__name_dict.keys() + self.__unique_name_dict.keys()

    def list_activatable_names(self):
        return self.__name_dict.keys() + self.__unique_name_dict.keys()

    def name_has_owner(self, name):
        if name.startswith(":"):
            return name in self.__unique_name_dict
        return name in self.__name_dict

    def get_name_owner(self, name):
        if name == dbus.BUS_DAEMON_NAME or \
            name == ibus.IBUS_NAME:
            return name
        if name.startswith(":"):
            if name in self.__unique_name_dict:
                return name
        elif name in self.__name_dict:
            ibusconn = self.__name_dict[name]
            return ibusconn.get_unique_name()

        raise ibus.IBusException(
                "org.freedesktop.DBus.Error.NameHasNoOwner:\n"
                "\tCould not get owner of name '%s': no such name" % name)

    def start_service_by_name(self, name, flags):
        return 0

    def get_connection_unix_user(self, connection_name):
        return 0

    def add_match(self, ibusconn, rule):
        self.__connections[ibusconn].append(MatchRule(rule))

    def remove_match(self, ibusconn, rule):
        pass

    def request_name(self, ibusconn, name, flags):
        if name.startswith(":"):
            raise ibus.IBusException("Only unique name can start with ':'")
        if not ibusconn.get_unique_name():
            raise ibus.IBusException("Can not call any method before Hello.")
        if name in ibusconn.get_names():
            return 0
        if name in self.__name_dict:
            raise ibus.IBusException("Name has been registered")
        self.__name_dict[name] = ibusconn
        self.emit("name-owner-changed", name, "", ibusconn.get_unique_name())
        ibusconn.add_name(name)
        return 1

    def release_name(self, ibusconn, name):
        if name.startswith(":"):
            raise ibus.IBusException("Only unique name can start with ':'")
        if name not in ibusconn.get_names():
            return 2
        del self.__name_dict[name]
        self.emit("name-owner-changed", name, ibusconn.get_unique_name(), "")
        ibusconn.remove_name(name)
        return 1

    def dispatch_dbus_signal(self, message):
        for conn, rules in self.__connections.items():
            for rule in rules:
                if rule.match_message(self, message):
                    conn.send_message(message)
                    break
    def get_connection_by_name(self, name):
        if not name.startswith(":"):
            return None
        return self.__unique_name_dict.get(name, None)

    def do_name_owner_changed(self, name, old_name, new_name):
        message = dbus.lowlevel.SignalMessage(dbus.BUS_DAEMON_PATH,
                        dbus.BUS_DAEMON_IFACE, "NameOwnerChanged")
        # message.set_sender(dbus.BUS_DAEMON_NAME)
        message.append(name)
        message.append(old_name)
        message.append(new_name)
        self.dispatch_dbus_signal(message)

    def __dbus_message_cb(self, ibusconn, message):
        dest = message.get_destination()
        message.set_sender(ibusconn.get_unique_name())
        if (not dest):
            if message.get_type() != 4: # Is not signal
                raise ibus.IBusException("Message without destination")
            self.dispatch_dbus_signal(message)
            return True

        if not dest.startswith(":"):
            raise ibus.IBusException("Destination of message must be an unique name")

        destconn = self.__unique_name_dict.get(dest, None)
        if destconn == None:
            raise ibus.IBusException("Can not find the destination(%s)" % dest)

        destconn.send_message(message)
        return True

    def __ibusconn_destroy_cb(self, ibusconn):
        name = ibusconn.get_unique_name()
        for name in ibusconn.get_names():
            del self.__name_dict[name]
            self.emit("name-owner-changed", name, ibusconn.get_unique_name(), "")
        if name:
            del self.__unique_name_dict[name]
            self.emit("name-owner-changed", name, name, "")

        del self.__connections[ibusconn]

bus = DBusReal()

class DBus(dbus.service.Object):

    method = lambda **args: \
        dbus.service.method(dbus_interface = dbus.BUS_DAEMON_IFACE, \
        **args)

    signal = lambda **args: \
        dbus.service.signal(dbus_interface = dbus.BUS_DAEMON_IFACE, \
        **args)

    __bus = bus

    def __init__(self, ibusconn):
        super(DBus, self).__init__(ibusconn.get_dbusconn(), dbus.BUS_DAEMON_PATH)
        self.__ibusconn = ibusconn
        self.__name = DBus.__bus.register_connection(self.__ibusconn)
        self.__active = False
        self.__bus.add_connection(ibusconn)

    @method(out_signature="s")
    def Hello(self):
        if not self.__active:
            self.__active = True
            return self.__name
        raise ibus.IBusException("Already handled an Hello message")

    @method(out_signature="as")
    def ListNames(self):
        return DBus.__bus.list_names()

    @method(out_signature="as")
    def ListActivatableNames(self):
        return DBus.__bus.list_activatable_names()

    @method(in_signature="s", out_signature="as")
    def NameHasOwner(self, name):
        return DBus.__bus.name_has_owner(name)

    @method(in_signature="si", out_signature="i")
    def StartServiceByName(self, name, flags):
        return DBus.__bus.start_service_by_name(name, flags)

    @method(in_signature="s", out_signature="s")
    def GetNameOwner(self, name):
        return DBus.__bus.get_name_owner(name)

    @method(in_signature="s", out_signature="i")
    def GetConnectionUnixUser(self, connection_name):
        return DBus.__bus.get_connection_unix_user(connection_name)

    @method(in_signature="s")
    def AddMatch(self, rule):
        return  DBus.__bus.add_match(self.__ibusconn, rule)

    @method(in_signature="s")
    def RemoveMatch(self, rule):
        return  DBus.__bus.remove_match(self.__ibusconn, rule)

    @method(out_signature="s")
    def GetId(self):
        return self.__name

    @method(in_signature="su", out_signature="u")
    def RequestName(self, name, flags):
        return self.__bus.request_name(self.__ibusconn, name, flags)

    @method(in_signature="s", out_signature="u")
    def ReleaseName(self, name):
        return self.__bus.release_name(self.__ibusconn, name)

    @signal(signature="sss")
    def NameOwnerChanged(self, name, old_owner, new_owner):
        pass

    @signal(signature="s")
    def NameLost(self, name):
        pass

    @signal(signature="s")
    def NameAcquired(self, name):
        pass
