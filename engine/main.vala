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

class DummyEngine : IBus.EngineSimple {
}

public int main(string[] args) {
    IBus.init();

    IBus.Bus bus = new IBus.Bus();
    if (!bus.is_connected()) {
        warning("ibus-daemon does not exist.");
        return 1;
    }

    bus.disconnected.connect((bus) => {
        debug("bus disconnected");
        IBus.quit();
    });

    IBus.Factory factory = new IBus.Factory(bus.get_connection());
    
    int id = 0;

    factory.create_engine.connect((factory, name) => {
        const string path = "/org/freedesktop/IBus/engine/simple/%d";
        IBus.Engine engine = new IBus.Engine.with_type(
            typeof(IBus.EngineSimple), name,
            path.printf(++id), bus.get_connection());
        return engine;
    });

    uint flags = 
        IBus.BusNameFlag.REPLACE_EXISTING |
        IBus.BusNameFlag.ALLOW_REPLACEMENT;
    uint retval = bus.request_name("org.freedesktop.IBus.Simple", flags);

    if (retval == 0) {
        warning("Registry bus name org.freedesktop.IBus.Simple failed!");
        return 1;
    }

    IBus.main();

    return 0;
}
