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


bool name_only = false;

class EngineList {
    public EngineDesc[] data = {};
}

IBus.Bus get_bus() {
    IBus.init();
    var bus = new IBus.Bus();
    return bus;
}

int list_engine(string[] argv) {
    const OptionEntry[] options =  {
        { "name-only", 0, 0, OptionArg.NONE, out name_only, "engine name only", "engine name only" },
        { null }
    };

    var option = new OptionContext("command [OPTIONS]");
    option.add_main_entries(options, "ibus");

    try {
        option.parse(ref argv);
    } catch (OptionError e) {
    }

    var bus = get_bus();

    var engines = bus.list_engines();

    if (name_only) {
        foreach (var engine in engines) {
            print("%s\n", engine.get_name());
        }
        return 0;
    }

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

int get_set_engine(string[] argv) {
    var bus = get_bus();
    string engine = null;
    if (argv.length > 1)
        engine = argv[1];

    if (engine == null) {
        var desc = bus.get_global_engine();
        if (desc == null)
            return -1;
        print("%s\n", desc.get_name());
        return 0;
    } else {
        if(!bus.set_global_engine(engine))
            return -1;
        var desc = bus.get_global_engine();
        if (desc == null)
            return -1;
        string cmdline = "setxkbmap %s".printf(desc.get_layout());
        try {
            if (!GLib.Process.spawn_command_line_sync(cmdline)) {
                warning("Switch xkb layout to %s failed.",
                    desc.get_layout());
            }
        } catch (GLib.SpawnError e) {
            warning("execute setxkblayout failed");
        }
        return 0;
    }
    return 0;
}

int message_watch(string[] argv) {
    return 0;
}

delegate int EntryFunc(string[] argv);

struct CommandEntry {
    string name;
    EntryFunc entry;
}

public int main(string[] argv) {
    const CommandEntry commands[]  = {
        { "engine", get_set_engine },
        { "list-engine", list_engine },
        { "watch", message_watch }
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

