/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright(c) 2011-2013 Peng Huang <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

using IBus;

class DummyEngine : IBus.EngineSimple {
}

public int main(string[] args) {
    Intl.setlocale(LocaleCategory.ALL, "");
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

        /* I think "c" + "'" is c with acute U+0107 and
         * "c" + "," is c with cedilla U+00E7 and they are
         * visually comprehensible. But pt-br people need
         * "c" + "'" is c with cedilla and I think the
         * cedilla_compose_seqs is needed for the specific keyboards
         * or regions.
         * X11 uses compose by locale:
         * In /usr/share/X11/locale/en_US.UTF-8/Compose ,
         * <Multi_key> <apostrophe> <c> : U0107
         * At the moment, LC_CTYPE is checked here.
         * I am not sure if cedilla_compose_seqs is needed for us layout.
         * FIXME: Need to provide the customization.
         */
        IBus.EngineSimple? simple = (IBus.EngineSimple ?) engine; 
        simple.add_table_by_locale(null);

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
