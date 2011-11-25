/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright(c) 2011 Peng Huang <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or(at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

using IBus;
using GLib;
using Gtk;

class Application {
    private IBus.Bus m_bus;
    private Panel m_panel;

    public Application(string[] argv) {
        IBus.init();
        Gtk.init(ref argv);

        m_bus = new IBus.Bus();
        m_panel = new Panel(m_bus);

        m_bus.connected.connect(bus_connected);
        m_bus.disconnected.connect(bus_disconnected);

        if (m_bus.is_connected()) {
            init();
        }
    }

    private void init() {
        DBusConnection connection = m_bus.get_connection();
        connection.signal_subscribe("org.freedesktop.DBus",
                                    "org.freedesktop.DBus",
                                    "NameAcquired",
                                    "/org/freedesktop/DBus",
                                    "org.freedesktop.IBus.Panel",
                                    DBusSignalFlags.NONE,
                                    this.bus_name_acquired);
        connection.signal_subscribe("org.freedesktop.DBus",
                                    "org.freedesktop.DBus",
                                    "NameLost",
                                    "/org/freedesktop/DBus",
                                    "org.freedesktop.IBus.Panel",
                                    DBusSignalFlags.NONE,
                                    this.bus_name_lost);

        m_bus.request_name("org.freedesktop.IBus.Panel", 2);
    }

    public int run() {
        Gtk.main();
        return 0;
    }

    private void bus_name_acquired(DBusConnection connection,
                                   string sender_name,
                                   string object_path,
                                   string interface_name,
                                   string signal_name,
                                   Variant parameters) {
        debug("signal_name = %s", signal_name);
    }

    private void bus_name_lost(DBusConnection connection,
                               string sender_name,
                               string object_path,
                               string interface_name,
                               string signal_name,
                               Variant parameters) {
        debug("signal_name = %s", signal_name);
    }

    private void bus_disconnected(IBus.Bus bus) {
        debug("connection is lost.");
        Gtk.main_quit();
    }

    private void bus_connected(IBus.Bus bus) {
        init();
    }

    public static void main(string[] argv) {
        Application app = new Application(argv);
        app.run();
    }
}
