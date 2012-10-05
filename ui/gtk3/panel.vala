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

class Panel : IBus.PanelService {
    private IBus.Bus m_bus;
    private IBus.Config m_config;
    private Gtk.StatusIcon m_status_icon;
    private Gtk.Menu m_ime_menu;
    private Gtk.Menu m_sys_menu;
    private IBus.EngineDesc[] m_engines = {};
    private CandidatePanel m_candidate_panel;
    private Switcher m_switcher;
    private PropertyManager m_property_manager;
    private GLib.Pid m_setup_pid = 0;
    private Gtk.AboutDialog m_about_dialog;
    private Gtk.CssProvider m_css_provider;
    private const string ACCELERATOR_SWITCH_IME_FOREWARD = "<Control>space";

    private uint m_switch_keysym = 0;
    private Gdk.ModifierType m_switch_modifiers = 0;

    public Panel(IBus.Bus bus) {
        GLib.assert(bus.is_connected());
        // Chain up base class constructor
        GLib.Object(connection : bus.get_connection(),
                    object_path : "/org/freedesktop/IBus/Panel");

        m_bus = bus;

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
        bind_switch_shortcut();

        m_property_manager = new PropertyManager();
        m_property_manager.property_activate.connect((k, s) => {
            property_activate(k, s);
        });

        state_changed();
    }

    ~Panel() {
        unbind_switch_shortcut();
    }

    private void bind_switch_shortcut() {
        var keybinding_manager = KeybindingManager.get_instance();

        var accelerator = ACCELERATOR_SWITCH_IME_FOREWARD;
        Gtk.accelerator_parse(accelerator,
                out m_switch_keysym, out m_switch_modifiers);

        // Map virtual modifiers to (i.e.Mod2, Mod3, ...)
        const Gdk.ModifierType VIRTUAL_MODIFIERS = (
                Gdk.ModifierType.SUPER_MASK |
                Gdk.ModifierType.HYPER_MASK |
                Gdk.ModifierType.META_MASK);
        if ((m_switch_modifiers & VIRTUAL_MODIFIERS) != 0) {
        // workaround a bug in gdk vapi vala > 0.18
        // https://bugzilla.gnome.org/show_bug.cgi?id=677559
#if VALA_0_18
            Gdk.Keymap.get_default().map_virtual_modifiers(
                    ref m_switch_modifiers);
#else
            if ((m_switch_modifiers & Gdk.ModifierType.SUPER_MASK) != 0)
                m_switch_modifiers |= Gdk.ModifierType.MOD4_MASK;
            if ((m_switch_modifiers & Gdk.ModifierType.HYPER_MASK) != 0)
                m_switch_modifiers |= Gdk.ModifierType.MOD4_MASK;
#endif
            m_switch_modifiers &= ~VIRTUAL_MODIFIERS;
        }

        if (m_switch_keysym == 0 && m_switch_modifiers == 0) {
            warning("Parse accelerator '%s' failed!", accelerator);
            return;
        }

        keybinding_manager.bind(m_switch_keysym, m_switch_modifiers,
                (e) => handle_engine_switch(e, false));

        // accelerator already has Shift mask
        if ((m_switch_modifiers & Gdk.ModifierType.SHIFT_MASK) != 0)
            return;

        keybinding_manager.bind(m_switch_keysym,
                m_switch_modifiers | Gdk.ModifierType.SHIFT_MASK,
                (e) => handle_engine_switch(e, true));
    }

    private void unbind_switch_shortcut() {
        var keybinding_manager = KeybindingManager.get_instance();

        if (m_switch_keysym == 0 && m_switch_modifiers == 0)
            return;

        keybinding_manager.unbind(m_switch_keysym, m_switch_modifiers);
        keybinding_manager.unbind(m_switch_keysym,
                m_switch_modifiers | Gdk.ModifierType.SHIFT_MASK);

        m_switch_keysym = 0;
        m_switch_modifiers = 0;
    }

    private void set_custom_font() {
        Gdk.Display display = Gdk.Display.get_default();
        Gdk.Screen screen = (display != null) ?
                display.get_default_screen() : null;

        if (screen == null) {
            warning("Could not open display.");
            return;
        }

        bool use_custom_font = false;
        GLib.Variant var_use_custom_font = m_config.get_value("panel",
                                                              "use_custom_font");

        if (var_use_custom_font != null) {
            use_custom_font = var_use_custom_font.get_boolean();
        }

        if (m_css_provider != null) {
            Gtk.StyleContext.remove_provider_for_screen(screen,
                                                        m_css_provider);
            m_css_provider = null;
        }

        if (use_custom_font == false) {
            return;
        }

        string font_name = null;
        GLib.Variant var_custom_font = m_config.get_value("panel",
                                                          "custom_font");
        if (var_custom_font != null) {
            font_name = var_custom_font.dup_string();
        }

        if (font_name == null) {
            warning("No config panel:custom_font.");
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

    public void set_config(IBus.Config config) {
        if (m_config != null) {
            m_config.value_changed.disconnect(config_value_changed_cb);
            m_config.watch(null, null);
            m_config = null;
        }

        m_config = config;
        if (m_config != null) {
            m_config.value_changed.connect(config_value_changed_cb);
            m_config.watch("general", "preload_engines");
            m_config.watch("general", "engines_order");
            m_config.watch("panel", "custom_font");
            m_config.watch("panel", "use_custom_font");
            update_engines(m_config.get_value("general", "preload_engines"),
                           m_config.get_value("general", "engines_order"));
        } else {
            update_engines(null, null);
        }

        set_custom_font();
    }

    private void switch_engine(int i, bool force = false) {
        GLib.assert(i >= 0 && i < m_engines.length);

        // Do not need switch
        if (i == 0 && !force)
            return;

        IBus.EngineDesc engine = m_engines[i];

        if (!m_bus.set_global_engine(engine.get_name())) {
            warning("Switch engine to %s failed.", engine.get_name());
            return;
        }
        // set xkb layout
        string cmdline = "setxkbmap %s".printf(engine.get_layout());
        try {
            if (!GLib.Process.spawn_command_line_sync(cmdline)) {
                warning("Switch xkb layout to %s failed.",
                    engine.get_layout());
            }
        } catch (GLib.SpawnError e) {
            warning("Execute setxkbmap failed: %s", e.message);
        }
    }

    private void config_value_changed_cb(IBus.Config config,
                                         string section,
                                         string name,
                                         Variant variant) {
        if (section == "general" && name == "preload_engines") {
            update_engines(variant, null);
            return;
        }

        if (section == "panel" && (name == "custom_font" ||
                                   name == "use_custom_font")) {
            set_custom_font();
            return;
        }
    }

    private void handle_engine_switch(Gdk.Event event, bool revert) {
        // Do not need switch IME
        if (m_engines.length <= 1)
            return;

        uint primary_modifiers =
            KeybindingManager.get_primary_modifier(event.key.state);

        bool pressed = KeybindingManager.primary_modifier_still_pressed(
                event, primary_modifiers);
        if (pressed) {
            int i = revert ? m_engines.length - 1 : 1;
            i = m_switcher.run(m_switch_keysym, m_switch_modifiers, event,
                    m_engines, i);
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

    private void update_engines(GLib.Variant? var_engines,
                                GLib.Variant? var_order) {
        string[] engine_names = null;

        if (var_engines != null)
            engine_names = var_engines.dup_strv();
        if (engine_names == null || engine_names.length == 0)
            engine_names = {"xkb:us::eng"};

        string[] order_names =
            (var_order != null) ? var_order.dup_strv() : null;

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
        } else {
            var current_engine = m_engines[0];
            m_engines = engines;
            int i;
            for (i = 0; i < m_engines.length; i++) {
                if (current_engine.get_name() == engines[i].get_name()) {
                    switch_engine(i);
                    return;
                }
            }
            switch_engine(0, true);
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

        int width, height;
        Gtk.icon_size_lookup(Gtk.IconSize.MENU, out width, out height);

        // Append IMEs
        foreach (var engine in m_engines) {
            var language = engine.get_language();
            var longname = engine.get_longname();
            var item = new Gtk.ImageMenuItem.with_label(
                "%s - %s".printf (IBus.get_language_name(language), longname));
            if (engine.get_icon() != "") {
                var icon = new IconWidget(engine.get_icon(), width);
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
    }

    public override void focus_out(string input_context_path) {
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
        var icon_name = "ibus-keyboard";

        var engine = m_bus.get_global_engine();
        if (engine != null)
            icon_name = engine.get_icon();

        if (icon_name[0] == '/')
            m_status_icon.set_from_file(icon_name);
        else
            m_status_icon.set_from_icon_name(icon_name);

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
        if (m_config != null)
            m_config.set_value("general",
                               "engines_order",
                               new GLib.Variant.strv(names));
    }
}
