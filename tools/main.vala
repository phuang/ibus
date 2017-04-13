/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright(c) 2013 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright(c) 2015-2017 Takao Fujiwara <takao.fujiwara1@gmail.com>
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

private const string[] IBUS_SCHEMAS = {
    "org.freedesktop.ibus.general",
    "org.freedesktop.ibus.general.hotkey",
    "org.freedesktop.ibus.panel",
};

bool name_only = false;
/* system() exists as a public API. */
bool is_system = false;
string cache_file = null;
string emoji_font = null;
string annotation_lang = null;

class EngineList {
    public IBus.EngineDesc[] data = {};
}

IBus.Bus? get_bus() {
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

    var option = new OptionContext();
    option.add_main_entries(options, Config.GETTEXT_PACKAGE);

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

int read_cache (string[] argv) {
    const OptionEntry[] options = {
        { "system", 0, 0, OptionArg.NONE, out is_system,
          N_("Read the system registry cache."), null },
        { "file", 0, 0, OptionArg.STRING, out cache_file,
          N_("Read the registry cache FILE."), "FILE" },
        { null }
    };

    var option = new OptionContext();
    option.add_main_entries(options, Config.GETTEXT_PACKAGE);

    try {
        option.parse(ref argv);
    } catch (OptionError e) {
        stderr.printf("%s\n", e.message);
        return Posix.EXIT_FAILURE;
    }

    var registry = new IBus.Registry();

    if (cache_file != null) {
        if (!registry.load_cache_file(cache_file)) {
            stderr.printf(_("The registry cache is invalid.\n"));
            return Posix.EXIT_FAILURE;
        }
    } else {
        if (!registry.load_cache(!is_system)) {
            stderr.printf(_("The registry cache is invalid.\n"));
            return Posix.EXIT_FAILURE;
        }
    }

    var output = new GLib.StringBuilder();
    registry.output(output, 1);

    print ("%s\n", output.str);
    return Posix.EXIT_SUCCESS;
}

int write_cache (string[] argv) {
    const OptionEntry[] options = {
        { "system", 0, 0, OptionArg.NONE, out is_system,
          N_("Write the system registry cache."), null },
        { "file", 0, 0, OptionArg.STRING, out cache_file,
          N_("Write the registry cache FILE."),
          "FILE" },
        { null }
    };

    var option = new OptionContext();
    option.add_main_entries(options, Config.GETTEXT_PACKAGE);

    try {
        option.parse(ref argv);
    } catch (OptionError e) {
        stderr.printf("%s\n", e.message);
        return Posix.EXIT_FAILURE;
    }

    var registry = new IBus.Registry();
    registry.load();

    if (cache_file != null) {
        return registry.save_cache_file(cache_file) ?
                Posix.EXIT_SUCCESS : Posix.EXIT_FAILURE;
    }

    return registry.save_cache(!is_system) ?
            Posix.EXIT_SUCCESS : Posix.EXIT_FAILURE;
}

int print_address(string[] argv) {
    string address = IBus.get_address();
    print("%s\n", address != null ? address : "(null)");
    return Posix.EXIT_SUCCESS;
}

int read_config(string[] argv) {
    var output = new GLib.StringBuilder();

    foreach (string schema in IBUS_SCHEMAS) {
        GLib.Settings settings = new GLib.Settings(schema);

        output.append_printf("SCHEMA: %s\n", schema);

        foreach (string key in settings.list_keys()) {
            GLib.Variant variant = settings.get_value(key);
            output.append_printf("  %s: %s\n", key, variant.print(true));
        }
    }
    print("%s", output.str);

    return Posix.EXIT_SUCCESS;
}

int reset_config(string[] argv) {
    print("%s\n", _("Resetting…"));

    foreach (string schema in IBUS_SCHEMAS) {
        GLib.Settings settings = new GLib.Settings(schema);

        print("SCHEMA: %s\n", schema);

        foreach (string key in settings.list_keys()) {
            print("  %s\n", key);
            settings.reset(key);
        }
    }

    GLib.Settings.sync();
    print("%s\n", _("Done"));

    return Posix.EXIT_SUCCESS;
}

#if EMOJI_DICT
int emoji_dialog(string[] argv) {
    string cmd = Config.LIBEXECDIR + "/ibus-ui-emojier";

    var file = File.new_for_path(cmd);
    if (!file.query_exists())
        cmd = "../ui/gtk3/ibus-ui-emojier";

    argv[0] = cmd;

    string[] env = Environ.get();

    try {
        // Non-blocking
        Process.spawn_async(null, argv, env,
                            SpawnFlags.SEARCH_PATH,
                            null, null);
    } catch (SpawnError e) {
        stderr.printf("%s\n", e.message);
        return Posix.EXIT_FAILURE;
    }

    return Posix.EXIT_SUCCESS;
}
#endif

int print_help(string[] argv) {
    print_usage(stdout);
    return Posix.EXIT_SUCCESS;
}

delegate int EntryFunc(string[] argv);

struct CommandEntry {
    unowned string name;
    unowned string description;
    unowned EntryFunc entry;
}

static const CommandEntry commands[]  = {
    { "engine", N_("Set or get engine"), get_set_engine },
    { "exit", N_("Exit ibus-daemon"), exit_daemon },
    { "list-engine", N_("Show available engines"), list_engine },
    { "watch", N_("(Not implemented)"), message_watch },
    { "restart", N_("Restart ibus-daemon"), restart_daemon },
    { "version", N_("Show version"), print_version },
    { "read-cache", N_("Show the content of registry cache"), read_cache },
    { "write-cache", N_("Create registry cache"), write_cache },
    { "address", N_("Print the D-Bus address of ibus-daemon"), print_address },
    { "read-config", N_("Show the configuration values"), read_config },
    { "reset-config", N_("Reset the configuration values"), reset_config },
#if EMOJI_DICT
    { "emoji", N_("Save emoji on dialog to clipboard "), emoji_dialog },
#endif
    { "help", N_("Show this information"), print_help }
};

static string program_name;

void print_usage(FileStream stream) {
    stream.printf(_("Usage: %s COMMAND [OPTION...]\n\n"), program_name);
    stream.printf(_("Commands:\n"));
    for (int i = 0; i < commands.length; i++) {
        stream.printf("  %-12s    %s\n",
                      commands[i].name,
                      GLib.dgettext(null, commands[i].description));
    }
}

public int main(string[] argv) {
    GLib.Intl.setlocale(GLib.LocaleCategory.ALL, "");
    GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE, Config.GLIB_LOCALE_DIR);
    GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE, "UTF-8");
    GLib.Intl.textdomain(Config.GETTEXT_PACKAGE);

    IBus.init();

    program_name = Path.get_basename(argv[0]);
    if (argv.length < 2) {
        print_usage(stderr);
        return Posix.EXIT_FAILURE;
    }

    string[] new_argv = argv[1:argv.length];
    new_argv[0] = "%s %s".printf(program_name, new_argv[0]);
    for (int i = 0; i < commands.length; i++) {
        if (commands[i].name == argv[1])
            return commands[i].entry(new_argv);
    }

    stderr.printf(_("%s is unknown command!\n"), argv[1]);
    print_usage(stderr);
    return Posix.EXIT_FAILURE;
}
