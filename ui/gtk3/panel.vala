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
    // private IBus.PanelService m_service;
    private Gtk.StatusIcon m_status_icon;
    private Gtk.Menu m_ime_menu;
    private IBus.EngineDesc[] m_engines;
    private CandidatePanel m_candidate_panel;

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

        KeybindingManager.get_instance().bind("<Control>space", (d) => {
            // Switch to next engine
            IBus.EngineDesc engine = m_bus.get_global_engine();
            int i;
            for (i = 0; i < m_engines.length && engine != null; i++) {
                if (m_engines[i].get_name() == engine.get_name())
                    break;
            }
            i ++;
            if (i >= m_engines.length) i = 0;
            if (i >= m_engines.length)
                return;
            m_bus.set_global_engine(m_engines[i].get_name());
        });
    }

    private void update_engines() {
        Variant variant = m_config.get_value("general", "preload_engines");
        if (variant != null)
            m_engines = m_bus.get_engines_by_names(variant.get_strv());
        else
            m_engines = m_bus.get_engines_by_names({"xkb:us:eng", "pinyin"});
        m_ime_menu = null;
    }

    private void status_icon_popup_menu(Gtk.StatusIcon status_icon,
                                        uint button,
                                        uint activate_time) {
        debug("popup-menu %u %u", button, activate_time);
    }

    private void status_icon_activate(Gtk.StatusIcon status_icon) {
        if (m_ime_menu == null) {
            int width, height;
            Gtk.icon_size_lookup(Gtk.IconSize.MENU, out width, out height);
            m_ime_menu  = new Gtk.Menu();
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
                item.activate.connect((i) => {
                    m_bus.set_global_engine(e.get_name());
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

    public override void set_cursor_location(int x, int y, int width, int height) {
        m_candidate_panel.set_cursor_location(x, y, width, height);
    }

    public override void focus_in(string input_context_path) {
        debug("focus_in ic=%s", input_context_path);
    }

    public override void focus_out(string input_context_path) {
        debug("focus_out ic=%s", input_context_path);
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

