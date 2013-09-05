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

class Panel : IBus.PanelService {
    private class Keybinding {
        public Keybinding(uint keysym,
                          Gdk.ModifierType modifiers,
                          bool reverse) {
            this.keysym = keysym;
            this.modifiers = modifiers;
            this.reverse = reverse;
        }

        public uint keysym { get; set; }
        public Gdk.ModifierType modifiers { get; set; }
        public bool reverse { get; set; }
    }

    private IBus.Bus m_bus;
    private GLib.Settings m_settings_general = null;
    private GLib.Settings m_settings_hotkey = null;
    private GLib.Settings m_settings_panel = null;
    private Gtk.StatusIcon m_status_icon;
    private Gtk.Menu m_ime_menu;
    private Gtk.Menu m_sys_menu;
    private IBus.EngineDesc[] m_engines = {};
    private GLib.HashTable<string, IBus.EngineDesc> m_engine_contexts =
            new GLib.HashTable<string, IBus.EngineDesc>(GLib.str_hash,
                                                        GLib.str_equal);
    private string m_current_context_path = "";
    private bool m_use_global_engine = true;
    private CandidatePanel m_candidate_panel;
    private Switcher m_switcher;
    private bool m_switcher_is_running = false;
    private PropertyManager m_property_manager;
    private GLib.Pid m_setup_pid = 0;
    private Gtk.AboutDialog m_about_dialog;
    private Gtk.CssProvider m_css_provider;
    private int m_switcher_delay_time = 400;
    private bool m_use_system_keyboard_layout = false;

    private GLib.List<Keybinding> m_keybindings = new GLib.List<Keybinding>();

    public Panel(IBus.Bus bus) {
        GLib.assert(bus.is_connected());
        // Chain up base class constructor
        GLib.Object(connection : bus.get_connection(),
                    object_path : "/org/freedesktop/IBus/Panel");

        m_bus = bus;

        init_settings();

        // init ui
        m_status_icon = new Gtk.StatusIcon();
        m_status_icon.set_name("ibus-ui-gtk");
        m_status_icon.set_title("IBus Panel");
        m_status_icon.popup_menu.connect(status_icon_popup_menu_cb);
        m_status_icon.activate.connect(status_icon_activate_cb);
        m_status_icon.set_from_icon_name("ibus-keyboard");

        m_candidate_panel = new CandidatePanel();
        m_candidate_panel.page_up.connect((w) => this.page_up());
        m_candidate_panel.page_down.connect((w) => this.page_down());

        m_switcher = new Switcher();
        // The initial shortcut is "<Super>space"
        bind_switch_shortcut();

        if (m_switcher_delay_time >= 0) {
            m_switcher.set_popup_delay_time((uint) m_switcher_delay_time);
        }

        m_property_manager = new PropertyManager();
        m_property_manager.property_activate.connect((k, s) => {
            property_activate(k, s);
        });

        state_changed();
    }

    ~Panel() {
        unbind_switch_shortcut();
    }

    private void init_settings() {
        m_settings_general = new GLib.Settings("org.freedesktop.ibus.general");
        m_settings_hotkey =
                new GLib.Settings("org.freedesktop.ibus.general.hotkey");
        m_settings_panel = new GLib.Settings("org.freedesktop.ibus.panel");

        m_settings_general.changed["preload-engines"].connect((key) => {
                update_engines(m_settings_general.get_strv(key),
                               null);
        });

        m_settings_general.changed["switcher-delay-time"].connect((key) => {
                set_switcher_delay_time();
        });

        m_settings_general.changed["use-system-keyboard-layout"].connect(
            (key) => {
                set_use_system_keyboard_layout();
        });

        m_settings_general.changed["embed-preedit-text"].connect((key) => {
                set_embed_preedit_text();
        });

        m_settings_general.changed["use-global-engine"].connect((key) => {
                set_use_global_engine();
        });

        m_settings_hotkey.changed["triggers"].connect((key) => {
                unbind_switch_shortcut();
                bind_switch_shortcut();
        });

        m_settings_panel.changed["custom-font"].connect((key) => {
                set_custom_font();
        });

        m_settings_panel.changed["use-custom-font"].connect((key) => {
                set_custom_font();
        });

        m_settings_panel.changed["show-icon-on-systray"].connect((key) => {
                set_show_icon_on_systray();
        });
    }

    private void keybinding_manager_bind(KeybindingManager keybinding_manager,
                                         string?           accelerator) {
        uint switch_keysym = 0;
        Gdk.ModifierType switch_modifiers = 0;
        Gdk.ModifierType reverse_modifier = Gdk.ModifierType.SHIFT_MASK;
        Keybinding keybinding;

        Gtk.accelerator_parse(accelerator,
                out switch_keysym, out switch_modifiers);

        // Map virtual modifiers to (i.e. Mod2, Mod3, ...)
        const Gdk.ModifierType VIRTUAL_MODIFIERS = (
                Gdk.ModifierType.SUPER_MASK |
                Gdk.ModifierType.HYPER_MASK |
                Gdk.ModifierType.META_MASK);
        if ((switch_modifiers & VIRTUAL_MODIFIERS) != 0) {
        // workaround a bug in gdk vapi vala > 0.18
        // https://bugzilla.gnome.org/show_bug.cgi?id=677559
#if VALA_0_18
            Gdk.Keymap.get_default().map_virtual_modifiers(
                    ref switch_modifiers);
#else
            if ((switch_modifiers & Gdk.ModifierType.SUPER_MASK) != 0)
                switch_modifiers |= Gdk.ModifierType.MOD4_MASK;
            if ((switch_modifiers & Gdk.ModifierType.HYPER_MASK) != 0)
                switch_modifiers |= Gdk.ModifierType.MOD4_MASK;
#endif
            switch_modifiers &= ~VIRTUAL_MODIFIERS;
        }

        if (switch_keysym == 0 && switch_modifiers == 0) {
            warning("Parse accelerator '%s' failed!", accelerator);
            return;
        }

        keybinding = new Keybinding(switch_keysym,
                                    switch_modifiers,
                                    false);
        m_keybindings.append(keybinding);

        keybinding_manager.bind(switch_keysym, switch_modifiers,
                (e) => handle_engine_switch(e, false));

        // accelerator already has Shift mask
        if ((switch_modifiers & reverse_modifier) != 0) {
            return;
        }

        switch_modifiers |= reverse_modifier;

        keybinding = new Keybinding(switch_keysym,
                                    switch_modifiers,
                                    true);
        m_keybindings.append(keybinding);

        keybinding_manager.bind(switch_keysym, switch_modifiers,
                (e) => handle_engine_switch(e, true));
    }

    private void bind_switch_shortcut() {
        string[] accelerators = m_settings_hotkey.get_strv("triggers");

        var keybinding_manager = KeybindingManager.get_instance();

        foreach (var accelerator in accelerators) {
            keybinding_manager_bind(keybinding_manager, accelerator);
        }
    }

    private void unbind_switch_shortcut() {
        var keybinding_manager = KeybindingManager.get_instance();

        unowned GLib.List<Keybinding> keybindings = m_keybindings;

        while (keybindings != null) {
            Keybinding keybinding = keybindings.data;

            keybinding_manager.unbind(keybinding.keysym,
                                      keybinding.modifiers);
            keybindings = keybindings.next;
        }

        m_keybindings = null;
    }

    private void set_custom_font() {
        Gdk.Display display = Gdk.Display.get_default();
        Gdk.Screen screen = (display != null) ?
                display.get_default_screen() : null;

        if (screen == null) {
            warning("Could not open display.");
            return;
        }

        bool use_custom_font = m_settings_panel.get_boolean("use-custom-font");

        if (m_css_provider != null) {
            Gtk.StyleContext.remove_provider_for_screen(screen,
                                                        m_css_provider);
            m_css_provider = null;
        }

        if (use_custom_font == false) {
            return;
        }

        string font_name = m_settings_panel.get_string("custom-font");

        if (font_name == null) {
            warning("No config panel:custom-font.");
            return;
        }

        string data_format = "GtkLabel { font: %s; }";
        string data = data_format.printf(font_name);
        m_css_provider = new Gtk.CssProvider();

        try {
            m_css_provider.load_from_data(data, -1);
        } catch (GLib.Error e) {
            warning("Failed css_provider_from_data: %s: %s", font_name,
                                                             e.message);
            return;
        }

        Gtk.StyleContext.add_provider_for_screen(screen,
                                                 m_css_provider,
                                                 Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    private void set_switcher_delay_time() {
        m_switcher_delay_time =
                m_settings_general.get_int("switcher-delay-time");

        if (m_switcher_delay_time >= 0) {
            m_switcher.set_popup_delay_time((uint) m_switcher_delay_time);
        }
    }

    private void set_use_system_keyboard_layout() {
        m_use_system_keyboard_layout =
                m_settings_general.get_boolean("use-system-keyboard-layout");
    }

    private void set_embed_preedit_text() {
        Variant variant =
                    m_settings_general.get_value("embed-preedit-text");

        if (variant == null) {
            return;
        }

        m_bus.set_ibus_property("EmbedPreeditText", variant);
    }

    private void set_use_global_engine() {
        m_use_global_engine =
                m_settings_general.get_boolean("use-global-engine");
    }

    private void set_show_icon_on_systray() {
        m_status_icon.set_visible(
                m_settings_panel.get_boolean("show-icon-on-systray"));
    }

    private int compare_versions(string version1, string version2) {
        string[] version1_list = version1.split(".");
        string[] version2_list = version2.split(".");
        int major1, minor1, micro1, major2, minor2, micro2;

        if (version1 == version2) {
            return 0;
        }

        // The initial dconf value of "version" is "".
        if (version1 == "") {
            return -1;
        }
        if (version2 == "") {
            return 1;
        }

        assert(version1_list.length >= 3);
        assert(version2_list.length >= 3);

        major1 = int.parse(version1_list[0]);
        minor1 = int.parse(version1_list[1]);
        micro1 = int.parse(version1_list[2]);

        major2 = int.parse(version2_list[0]);
        minor2 = int.parse(version2_list[1]);
        micro2 = int.parse(version2_list[2]);

        if (major1 == minor1 && minor1 == minor2 && micro1 == micro2) {
            return 0;
        }
        if ((major1 > major2) ||
            (major1 == major2 && minor1 > minor2) ||
            (major1 == major2 && minor1 == minor2 &&
             micro1 > micro2)) {
            return 1;
        }
        return -1;
    }

    private void update_version_1_5_3() {
#if ENABLE_LIBNOTIFY
        if (!Notify.is_initted()) {
            Notify.init ("ibus");
        }

        var notification = new Notify.Notification(
                _("IBus Update"),
                _("Super+space is now the default hotkey."),
                "ibus");
        notification.set_timeout(30 * 1000);
        notification.set_category("hotkey");

        try {
            notification.show();
        } catch (GLib.Error e){
            warning ("Notification is failed for IBus 1.5.3: %s", e.message);
        }
#else
        warning(_("Super+space is now the default hotkey."));
#endif
    }

    private void set_version() {
        string prev_version = m_settings_general.get_string("version");
        string current_version = null;

        if (compare_versions(prev_version, "1.5.3") < 0) {
            update_version_1_5_3();
        }

        current_version = "%d.%d.%d".printf(IBus.MAJOR_VERSION,
                                            IBus.MINOR_VERSION,
                                            IBus.MICRO_VERSION);

        if (prev_version == current_version) {
            return;
        }

        m_settings_general.set_string("version", current_version);
    }

    public void load_settings() {
        // Update m_use_system_keyboard_layout before update_engines()
        // is called.
        set_use_system_keyboard_layout();
        set_use_global_engine();
        update_engines(m_settings_general.get_strv("preload-engines"),
                       m_settings_general.get_strv("engines-order"));
        unbind_switch_shortcut();
        bind_switch_shortcut();
        set_switcher_delay_time();
        set_embed_preedit_text();
        set_custom_font();
        set_show_icon_on_systray();

        set_version();
    }

    private void exec_setxkbmap(IBus.EngineDesc engine) {
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
            return;
        }

        try {
            if (!GLib.Process.spawn_sync(null, args, null,
                                         GLib.SpawnFlags.SEARCH_PATH,
                                         null, null,
                                         out standard_error,
                                         out exit_status)) {
                warning("Switch xkb layout to %s failed.",
                        engine.get_layout());
            }
        } catch (GLib.SpawnError e) {
            warning("Execute setxkbmap failed: %s", e.message);
        }

        if (exit_status != 0) {
            warning("Execute setxkbmap failed: %s", standard_error ?? "(null)");
        }
    }

    private void engine_contexts_insert(IBus.EngineDesc engine) {
        if (m_use_global_engine)
            return;

        if (m_engine_contexts.size() >= 200) {
            warning ("Contexts by windows are too much counted!");
            m_engine_contexts.remove_all();
        }

        m_engine_contexts.replace(m_current_context_path, engine);
    }

    private void set_engine(IBus.EngineDesc engine) {
        if (!m_bus.set_global_engine(engine.get_name())) {
            warning("Switch engine to %s failed.", engine.get_name());
            return;
        }

        // set xkb layout
        if (!m_use_system_keyboard_layout)
            exec_setxkbmap(engine);

        engine_contexts_insert(engine);
    }

    private void switch_engine(int i, bool force = false) {
        GLib.assert(i >= 0 && i < m_engines.length);

        // Do not need switch
        if (i == 0 && !force)
            return;

        IBus.EngineDesc engine = m_engines[i];

        set_engine(engine);
    }

    private void handle_engine_switch(Gdk.Event event, bool revert) {
        // Do not need switch IME
        if (m_engines.length <= 1)
            return;

        uint keyval = event.key.keyval;
        uint modifiers = KeybindingManager.MODIFIER_FILTER & event.key.state;

        uint primary_modifiers =
            KeybindingManager.get_primary_modifier(event.key.state);

        bool pressed = KeybindingManager.primary_modifier_still_pressed(
                event, primary_modifiers);

        if (revert) {
            modifiers &= ~Gdk.ModifierType.SHIFT_MASK;
        }

        if (pressed && m_switcher_delay_time >= 0) {
            int i = revert ? m_engines.length - 1 : 1;

            /* The flag of m_switcher_is_running avoids the following problem:
             *
             * When an IME is chosen on m_switcher, focus_in() is called
             * for the root window. If an engine is set in focus_in()
             * during running m_switcher when m_use_global_engine is false,
             * state_changed() is also called and m_engines[] is modified
             * in state_changed() and m_switcher.run() returns the index
             * for m_engines[] but m_engines[] was modified by state_changed()
             * and the index is not correct. */
            m_switcher_is_running = true;
            i = m_switcher.run(keyval, modifiers, event, m_engines, i);
            m_switcher_is_running = false;

            if (i < 0) {
                debug("switch cancelled");
            } else {
                GLib.assert(i < m_engines.length);
                switch_engine(i);
            }
        } else {
            int i = revert ? m_engines.length - 1 : 1;
            switch_engine(i);
        }
    }

    private void run_preload_engines(IBus.EngineDesc[] engines, int index) {
        string[] names = {};

        if (engines.length <= index) {
            return;
        }

        names += engines[index].get_name();
        m_bus.preload_engines_async(names, -1, null);
    }

    private void update_engines(string[]? unowned_engine_names,
                                string[]? order_names) {
        string[]? engine_names = unowned_engine_names;

        if (engine_names == null || engine_names.length == 0)
            engine_names = {"xkb:us::eng"};

        string[] names = {};

        foreach (var name in order_names) {
            if (name in engine_names)
                names += name;
        }

        foreach (var name in engine_names) {
            if (name in names)
                continue;
            names += name;
        }

        var engines = m_bus.get_engines_by_names(names);

        if (m_engines.length == 0) {
            m_engines = engines;
            switch_engine(0, true);
            run_preload_engines(engines, 1);
        } else {
            var current_engine = m_engines[0];
            m_engines = engines;
            int i;
            for (i = 0; i < m_engines.length; i++) {
                if (current_engine.get_name() == engines[i].get_name()) {
                    switch_engine(i);
                    if (i != 0) {
                        run_preload_engines(engines, 0);
                    } else {
                        run_preload_engines(engines, 1);
                    }
                    return;
                }
            }
            switch_engine(0, true);
            run_preload_engines(engines, 1);
        }

    }

    private void show_setup_dialog() {
        if (m_setup_pid != 0) {
            if (Posix.kill(m_setup_pid, Posix.SIGUSR1) == 0)
                return;
            m_setup_pid = 0;
        }

        string binary = GLib.Path.build_filename(Config.BINDIR, "ibus-setup");
        try {
            GLib.Process.spawn_async(null,
                                     {binary, "ibus-setup"},
                                     null,
                                     GLib.SpawnFlags.DO_NOT_REAP_CHILD,
                                     null,
                                     out m_setup_pid);
        } catch (GLib.SpawnError e) {
            warning("Execute %s failed! %s", binary, e.message);
            m_setup_pid = 0;
        }

        GLib.ChildWatch.add(m_setup_pid, (pid, state) => {
            if (pid != m_setup_pid)
                return;
            m_setup_pid = 0;
            GLib.Process.close_pid(pid);
        });
    }

    private void show_about_dialog() {
        if (m_about_dialog == null) {
            m_about_dialog = new Gtk.AboutDialog();
            m_about_dialog.set_program_name("IBus");
            m_about_dialog.set_version(Config.PACKAGE_VERSION);

            string copyright = _(
                "Copyright (c) 2007-2012 Peng Huang\n" +
                "Copyright (c) 2007-2010 Red Hat, Inc.\n");

            m_about_dialog.set_copyright(copyright);
            m_about_dialog.set_license("LGPL");
            m_about_dialog.set_comments(_("IBus is an intelligent input bus for Linux/Unix."));
            m_about_dialog.set_website("http://code.google.com/p/ibus");
            m_about_dialog.set_authors({"Peng Huang <shawn.p.huang@gmail.com>"});
            m_about_dialog.set_documenters({"Peng Huang <shawn.p.huang@gmail.com>"});
            m_about_dialog.set_translator_credits(_("translator-credits"));
            m_about_dialog.set_logo_icon_name("ibus");
            m_about_dialog.set_icon_name("ibus");
        }

        if (!m_about_dialog.get_visible()) {
            m_about_dialog.run();
            m_about_dialog.hide();
        } else {
            m_about_dialog.present();
        }
    }

    private void status_icon_popup_menu_cb(Gtk.StatusIcon status_icon,
                                           uint button,
                                           uint activate_time) {
        // Show system menu
        if (m_sys_menu == null) {
            Gtk.ImageMenuItem item;
            m_sys_menu = new Gtk.Menu();

            item = new Gtk.ImageMenuItem.from_stock(Gtk.Stock.PREFERENCES, null);
            item.activate.connect((i) => show_setup_dialog());
            m_sys_menu.append(item);

            item = new Gtk.ImageMenuItem.from_stock(Gtk.Stock.ABOUT, null);
            item.activate.connect((i) => show_about_dialog());
            m_sys_menu.append(item);

            m_sys_menu.append(new Gtk.SeparatorMenuItem());

            item = new Gtk.ImageMenuItem.from_stock(Gtk.Stock.REFRESH, null);
            item.set_label(_("Restart"));
            item.activate.connect((i) => m_bus.exit(true));
            m_sys_menu.append(item);

            item = new Gtk.ImageMenuItem.from_stock(Gtk.Stock.QUIT, null);
            item.activate.connect((i) => m_bus.exit(false));
            m_sys_menu.append(item);

            m_sys_menu.show_all();
        }

        m_sys_menu.popup(null,
                         null,
                         m_status_icon.position_menu,
                         0,
                         Gtk.get_current_event_time());
    }

    private void status_icon_activate_cb(Gtk.StatusIcon status_icon) {
        m_ime_menu = new Gtk.Menu();

        // Show properties and IME switching menu
        m_property_manager.create_menu_items(m_ime_menu);

        m_ime_menu.append(new Gtk.SeparatorMenuItem());

        // Append IMEs
        foreach (var engine in m_engines) {
            var language = engine.get_language();
            var longname = engine.get_longname();
            var item = new Gtk.ImageMenuItem.with_label(
                "%s - %s".printf (IBus.get_language_name(language), longname));
            if (engine.get_icon() != "") {
                var icon = new IconWidget(engine.get_icon(), Gtk.IconSize.MENU);
                 item.set_image(icon);
            }
            // Make a copy of engine to workaround a bug in vala.
            // https://bugzilla.gnome.org/show_bug.cgi?id=628336
            var e = engine;
            item.activate.connect((item) => {
                for (int i = 0; i < m_engines.length; i++) {
                    if (e == m_engines[i]) {
                        switch_engine(i);
                        break;
                    }
                }
            });
            m_ime_menu.add(item);
        }

        m_ime_menu.show_all();

        // Do not take focuse to avoid some focus related issues.
        m_ime_menu.set_take_focus(false);
        m_ime_menu.popup(null,
                         null,
                         m_status_icon.position_menu,
                         0,
                         Gtk.get_current_event_time());
    }

    /* override virtual functions */
    public override void set_cursor_location(int x, int y,
                                             int width, int height) {
        m_candidate_panel.set_cursor_location(x, y, width, height);
    }

    public override void focus_in(string input_context_path) {
        if (m_use_global_engine)
            return;

        /* Do not change the order of m_engines during running switcher. */
        if (m_switcher_is_running)
            return;

        m_current_context_path = input_context_path;

        var engine = m_engine_contexts[m_current_context_path];

        if (engine == null) {
            /* If engine == null, need to call set_engine(m_engines[0])
             * here and update m_engine_contexts[] to avoid the
             * following problem:
             *
             * If context1 is focused and does not set an engine and
             * return here, the current engine1 is used for context1.
             * When context2 is focused and switch engine1 to engine2,
             * the current engine becomes engine2.
             * And when context1 is focused again, context1 still
             * does not set an engine and return here,
             * engine2 is used for context2 instead of engine1. */
            engine = m_engines.length > 0 ? m_engines[0] : null;

            if (engine == null)
                return;
        } else {
            bool in_engines = false;

            foreach (var e in m_engines) {
                if (engine.get_name() == e.get_name()) {
                    in_engines = true;
                    break;
                }
            }

            /* The engine is deleted by ibus-setup before focus_in()
             * is called. */
            if (!in_engines)
                return;
        }

        set_engine(engine);
    }

    public override void focus_out(string input_context_path) {
        if (m_use_global_engine)
            return;

        /* Do not change the order of m_engines during running switcher. */
        if (m_switcher_is_running)
            return;

        m_current_context_path = "";
    }

    public override void destroy_context(string input_context_path) {
        if (m_use_global_engine)
            return;

        m_engine_contexts.remove(input_context_path);
    }

    public override void register_properties(IBus.PropList props) {
        m_property_manager.set_properties(props);
    }

    public override void update_property(IBus.Property prop) {
        m_property_manager.update_property(prop);
    }

    public override void update_preedit_text(IBus.Text text,
                                             uint cursor_pos,
                                             bool visible) {
        if (visible)
            m_candidate_panel.set_preedit_text(text, cursor_pos);
        else
            m_candidate_panel.set_preedit_text(null, 0);
    }

    public override void hide_preedit_text() {
        m_candidate_panel.set_preedit_text(null, 0);
    }

    public override void update_auxiliary_text(IBus.Text text,
                                               bool visible) {
        m_candidate_panel.set_auxiliary_text(visible ? text : null);
    }

    public override void hide_auxiliary_text() {
        m_candidate_panel.set_auxiliary_text(null);
    }

    public override void update_lookup_table(IBus.LookupTable table,
                                             bool visible) {
        m_candidate_panel.set_lookup_table(visible ? table : null);
    }

    public override void hide_lookup_table() {
        m_candidate_panel.set_lookup_table(null);
    }

    public override void state_changed() {
        /* Do not change the order of m_engines during running switcher. */
        if (m_switcher_is_running)
            return;

        var icon_name = "ibus-keyboard";

        var engine = m_bus.get_global_engine();
        if (engine != null)
            icon_name = engine.get_icon();

        if (icon_name[0] == '/')
            m_status_icon.set_from_file(icon_name);
        else {
            var theme = Gtk.IconTheme.get_default();
            if (theme.lookup_icon(icon_name, 48, 0) != null) {
                m_status_icon.set_from_icon_name(icon_name);
            } else {
                m_status_icon.set_from_icon_name("ibus-engine");
            }
        }

        if (engine == null)
            return;

        int i;
        for (i = 0; i < m_engines.length; i++) {
            if (m_engines[i].get_name() == engine.get_name())
                break;
        }

        // engine is first engine in m_engines.
        if (i == 0)
            return;

        // engine is not in m_engines.
        if (i >= m_engines.length)
            return;

        for (int j = i; j > 0; j--) {
            m_engines[j] = m_engines[j - 1];
        }
        m_engines[0] = engine;

        string[] names = {};
        foreach(var desc in m_engines) {
            names += desc.get_name();
        }
        m_settings_general.set_strv("engines-order", names);
    }
}
