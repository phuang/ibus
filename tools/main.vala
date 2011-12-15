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
using GLib;
using IBus;


string opt1 = null;

class EngineList {
    public EngineDesc[] data = {};
}

int list_engine(string[] argv) {
    const OptionEntry[] options =  {
        { "opt1", 0, 0, OptionArg.STRING, out opt1, "opt1 desc", "opt2 short desc" },
        { null }
    };

    var option = new OptionContext("command [OPTIONS]");
    option.add_main_entries(options, "ibus");

    try {
        option.parse(ref argv);
    } catch (OptionError e) {
    }

    IBus.init();
    var bus = new IBus.Bus();

    var engines = bus.list_engines();

    var map = new HashTable<string, EngineList>(GLib.str_hash, GLib.str_equal);

    foreach (var engine in engines) {
        var list = map.get(engine.get_language());
        if (list == null) {
            list = new EngineList();
            map.insert(engine.get_language(), list);
        }
        list.data += engine;
    }

    foreach (var language in map.get_keys()) {
        var list = map.get(language);
        print("language: %s\n", IBus.get_language_name(language));
        foreach (var engine in list.data) {
            print("  %s - %s\n", engine.get_name(), engine.get_longname());
        }
    }

    return 0;
}

int switch_engine(string[] argv) {
    IBus.init();
    var bus = new IBus.Bus();
    bus.set_global_engine(argv[1]);
    return 0;
}

delegate int EntryFunc(string[] argv);

struct CommandEntry {
    string name;
    EntryFunc entry;
}

public int main(string[] argv) {
    const CommandEntry commands[]  = {
        { "list-engine", list_engine },
        { "switch-engine", switch_engine }
    };

    if (argv.length >= 2) {
        string[] new_argv = argv[1:argv.length];
        foreach (var command in commands) {
            if (command.name == argv[1])
                return command.entry(new_argv);
        }
        warning("%s is unknown command!", argv[1]);
    }

    return -1;
}

