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

class Panel : IBus.PanelService {
    private IBus.Bus m_bus;
    private IBus.Config m_config;
    private Gtk.StatusIcon m_status_icon;
    private Gtk.Menu m_ime_menu;
    private IBus.EngineDesc[] m_engines;
    private CandidatePanel m_candidate_panel;
    private Switcher m_switcher;
    private PropertyManager m_property_manager;
    private IBus.InputContext m_input_context;

    public Panel(IBus.Bus bus) {
        assert(bus.is_connected());
        // Chain up base class constructor
        GLib.Object(connection : bus.get_connection(),
                    object_path : "/org/freedesktop/IBus/Panel");

        m_bus = bus;

        m_config = bus.get_config();

        // init ui
        m_status_icon = new Gtk.StatusIcon();
        m_status_icon.set_name("ibus-ui-gtk");
        m_status_icon.set_title("IBus Panel");
        m_status_icon.popup_menu.connect(status_icon_popup_menu);
        m_status_icon.activate.connect(status_icon_activate);
        m_status_icon.set_from_icon_name("ibus-keyboard");

        m_candidate_panel = new CandidatePanel();

        m_candidate_panel.hide();
        m_candidate_panel.show();

        update_engines();

        m_switcher = new Switcher();

        KeybindingManager.get_instance().bind("<Control>space", (e) => {
            handle_engine_switch(e, false);
        });

        KeybindingManager.get_instance().bind("<Control><Shift>space", (e) => {
            handle_engine_switch(e, true);
        });

        m_property_manager = new PropertyManager();
        m_property_manager.property_activate.connect((k, s) => {
            if (m_input_context != null)
                m_input_context.property_activate(k, s);
        });
    }

    private void switch_engine(int i) {
        //  debug("switch_engine i = %d", i);
        assert(i >= 0 && i < m_engines.length);

        // Do not need siwtch
        if (i == 0)
            return;

        // Move the target engine to the first place.
        IBus.EngineDesc engine = m_engines[i];
        for (int j = i; j > 0; j--) {
            m_engines[j] = m_engines[j - 1];
        }
        m_engines[0] = engine;

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
            warning("execute setxkblayout failed");
        }
    }

    private void handle_engine_switch(Gdk.Event event, bool revert) {
        if (!KeybindingManager.primary_modifier_still_pressed(event)) {
            int i = revert ? m_engines.length - 1 : 1;
            switch_engine(i);
        } else {
            int i = revert ? m_engines.length - 1 : 1;
            i = m_switcher.run(event, m_engines, i);
            if (i < 0) {
                debug("switch cancelled");
            } else {
                assert(i < m_engines.length);
                switch_engine(i);
            }
        }
    }

    private void update_engines() {
        Variant variant = m_config.get_value("general", "preload_engines");
        string[] engine_names;

        if (variant != null)
            engine_names = variant.dup_strv();
        else
            engine_names = {"xkb:layout:us", "pinyin", "anthy"};

        m_engines = m_bus.get_engines_by_names(engine_names);
    }

    private void status_icon_popup_menu(Gtk.StatusIcon status_icon,
                                        uint button,
                                        uint activate_time) {
        Gtk.Menu menu = m_property_manager.get_menu();
        if (menu == null)
            return;

        menu.show_all();
        menu.set_take_focus(false);

        menu.popup(null,
                   null,
                   m_status_icon.position_menu,
                   0,
                   Gtk.get_current_event_time());
    }

    private void status_icon_activate(Gtk.StatusIcon status_icon) {
        int width, height;
        Gtk.icon_size_lookup(Gtk.IconSize.MENU, out width, out height);
        if (m_ime_menu == null) {
            m_ime_menu = new Gtk.Menu();
            foreach (var engine in m_engines) {
                var lang =  engine.get_language();
                var name = engine.get_name();
                var item = new Gtk.ImageMenuItem.with_label(lang + " - " + name);
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
            m_ime_menu.set_take_focus(false);
        }
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
        try {
            GLib.Cancellable cancellable = null;
            m_input_context =
                new IBus.InputContext(input_context_path,
                                      m_bus.get_connection(),
                                      cancellable);
        } catch (GLib.Error e) {
            debug("error");
        }
    }

    public override void focus_out(string input_context_path) {
        m_input_context = null;
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
}
