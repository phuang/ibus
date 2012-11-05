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

bool name_only = false;

class EngineList {
    public IBus.EngineDesc[] data = {};
}

IBus.Bus? get_bus() {
    IBus.init();
    var bus = new IBus.Bus();
    if (!bus.is_connected ())
        return null;
    return bus;
}

int list_engine(string[] argv) {
    const OptionEntry[] options = {
        { "name-only", 0, 0, OptionArg.NONE, out name_only,
          N_("List engine name only"), null },
        { null }
    };

    var option = new OptionContext(_("command [OPTIONS]"));
    option.add_main_entries(options, "ibus");

    try {
        option.parse(ref argv);
    } catch (OptionError e) {
        stderr.printf("%s\n", e.message);
        return Posix.EXIT_FAILURE;
    }

    var bus = get_bus();
    if (bus == null) {
        stderr.printf(_("Can't connect to IBus.\n"));
        return Posix.EXIT_FAILURE;
    }

    var engines = bus.list_engines();

    if (name_only) {
        foreach (var engine in engines) {
            print("%s\n", engine.get_name());
        }
        return Posix.EXIT_SUCCESS;
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
        print(_("language: %s\n"), IBus.get_language_name(language));
        foreach (var engine in list.data) {
            print("  %s - %s\n", engine.get_name(), engine.get_longname());
        }
    }

    return Posix.EXIT_SUCCESS;
}

private int exec_setxkbmap(IBus.EngineDesc engine) {
    string layout = engine.get_layout();
    string variant = engine.get_layout_variant();
    string option = engine.get_layout_option();
    string standard_error = null;
    int exit_status = 0;
    string[] args = { "setxkbmap" };

    if (layout != null && layout != "" && layout != "default") {
        args += "-layout";
        args += layout;
    }
    if (variant != null && variant != "" && variant != "default") {
        args += "-variant";
        args += variant;
    }
    if (option != null && option != "" && option != "default") {
        /*TODO: Need to get the session XKB options */
        args += "-option";
        args += "-option";
        args += option;
    }

    if (args.length == 1) {
        return Posix.EXIT_FAILURE;
    }

    try {
        if (!GLib.Process.spawn_sync(null, args, null,
                                     GLib.SpawnFlags.SEARCH_PATH,
                                     null, null,
                                     out standard_error,
                                     out exit_status)) {
            warning("Switch xkb layout to %s failed.",
                    engine.get_layout());
            return Posix.EXIT_FAILURE;
        }
    } catch (GLib.SpawnError e) {
        warning("Execute setxkbmap failed: %s", e.message);
        return Posix.EXIT_FAILURE;
    }

    if (exit_status != 0) {
        warning("Execute setxkbmap failed: %s", standard_error ?? "(null)");
        return Posix.EXIT_FAILURE;
    }

    return Posix.EXIT_SUCCESS;
}

int get_set_engine(string[] argv) {
    var bus = get_bus();
    string engine = null;
    if (argv.length > 1)
        engine = argv[1];

    if (engine == null) {
        var desc = bus.get_global_engine();
        if (desc == null) {
            stderr.printf(_("No engine is set.\n"));
            return Posix.EXIT_FAILURE;
        }
        print("%s\n", desc.get_name());
        return Posix.EXIT_SUCCESS;
    }

    if(!bus.set_global_engine(engine)) {
        stderr.printf(_("Set global engine failed.\n"));
        return Posix.EXIT_FAILURE;
    }
    var desc = bus.get_global_engine();
    if (desc == null) {
        stderr.printf(_("Get global engine failed.\n"));
        return Posix.EXIT_FAILURE;
    }

    return exec_setxkbmap(desc);
}

int message_watch(string[] argv) {
    return Posix.EXIT_SUCCESS;
}

int restart_daemon(string[] argv) {
    var bus = get_bus();
    if (bus == null) {
        stderr.printf(_("Can't connect to IBus.\n"));
        return Posix.EXIT_FAILURE;
    }
    bus.exit(true);
    return Posix.EXIT_SUCCESS;
}

int exit_daemon(string[] argv) {
    var bus = get_bus();
    if (bus == null) {
        stderr.printf(_("Can't connect to IBus.\n"));
        return Posix.EXIT_FAILURE;
    }
    bus.exit(false);
    return Posix.EXIT_SUCCESS;
}

int print_version(string[] argv) {
    print("IBus %s\n", Config.PACKAGE_VERSION);
    return Posix.EXIT_SUCCESS;
}

int print_help(string[] argv) {
    print_usage(stdout);
    return Posix.EXIT_SUCCESS;
}

delegate int EntryFunc(string[] argv);

struct CommandEntry {
    string name;
    EntryFunc entry;
}

static const CommandEntry commands[]  = {
    { "engine", get_set_engine },
    { "exit", exit_daemon },
    { "list-engine", list_engine },
    { "watch", message_watch },
    { "restart", restart_daemon },
    { "version", print_version },
    { "help", print_help }
};

static string program_name;

void print_usage(FileStream stream) {
    stream.printf(_("Usage: %s COMMAND [OPTION...]\n\n"), program_name);
    stream.printf(_("Commands:\n"));
    for (int i = 0; i < commands.length; i++) {
        stream.printf("  %s\n", commands[i].name);
    }
}

public int main(string[] argv) {
    GLib.Intl.bindtextdomain (Config.GETTEXT_PACKAGE, Config.GLIB_LOCALE_DIR);
    GLib.Intl.bind_textdomain_codeset (Config.GETTEXT_PACKAGE, "UTF-8");

    program_name = Path.get_basename(argv[0]);
    if (argv.length < 2) {
        print_usage(stderr);
        return Posix.EXIT_FAILURE;
    }

    string[] new_argv = argv[1:argv.length];
    for (int i = 0; i < commands.length; i++) {
        if (commands[i].name == argv[1])
            return commands[i].entry(new_argv);
    }

    stderr.printf(_("%s is unknown command!\n"), argv[1]);
    print_usage(stderr);
    return Posix.EXIT_FAILURE;
}
