/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright(c) 2018 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright(c) 2018 Takao Fujwiara <takao.fujiwara1@gmail.com>
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

class PanelBinding : IBus.PanelService {
    private IBus.Bus m_bus;
    private GLib.Settings m_settings_panel = null;
    private GLib.Settings m_settings_emoji = null;
    private string m_current_context_path = "";
    private string m_real_current_context_path = "";
    private IBusEmojier? m_emojier;
    private uint m_emojier_set_emoji_lang_id;
    private uint m_emojier_focus_commit_text_id;
    private string[] m_emojier_favorites = {};
    private Gtk.CssProvider m_css_provider;
    private const uint PRELOAD_ENGINES_DELAY_TIME = 30000;
    private GLib.List<BindingCommon.Keybinding> m_keybindings =
            new GLib.List<BindingCommon.Keybinding>();

    public PanelBinding(IBus.Bus bus) {
        GLib.assert(bus.is_connected());
        // Chain up base class constructor
        GLib.Object(connection : bus.get_connection(),
                    object_path : IBus.PATH_PANEL_EXTENSION);

        m_bus = bus;

        init_settings();

        bind_emoji_shortcut();
    }


    ~PanelBinding() {
        BindingCommon.unbind_switch_shortcut(
                BindingCommon.KeyEventFuncType.ANY,
                m_keybindings);
    }


    private void init_settings() {
        m_settings_panel = new GLib.Settings("org.freedesktop.ibus.panel");
        m_settings_emoji = new GLib.Settings("org.freedesktop.ibus.panel.emoji");

        m_settings_panel.changed["custom-font"].connect((key) => {
                BindingCommon.set_custom_font(m_settings_panel,
                                              m_settings_emoji,
                                              ref m_css_provider);
        });

        m_settings_panel.changed["use-custom-font"].connect((key) => {
                BindingCommon.set_custom_font(m_settings_panel,
                                              m_settings_emoji,
                                              ref m_css_provider);
        });

        m_settings_emoji.changed["hotkey"].connect((key) => {
                BindingCommon.unbind_switch_shortcut(
                        BindingCommon.KeyEventFuncType.EMOJI_TYPING,
                        m_keybindings);
                bind_emoji_shortcut();
        });

        m_settings_emoji.changed["font"].connect((key) => {
                BindingCommon.set_custom_font(m_settings_panel,
                                              m_settings_emoji,
                                              ref m_css_provider);
        });

        m_settings_emoji.changed["favorites"].connect((key) => {
                set_emoji_favorites();
        });

        m_settings_emoji.changed["favorite-annotations"].connect((key) => {
                set_emoji_favorites();
        });

        m_settings_emoji.changed["lang"].connect((key) => {
                set_emoji_lang();
        });

        m_settings_emoji.changed["has-partial-match"].connect((key) => {
                set_emoji_partial_match();
        });

        m_settings_emoji.changed["partial-match-length"].connect((key) => {
                set_emoji_partial_match();
        });

        m_settings_emoji.changed["partial-match-condition"].connect((key) => {
                set_emoji_partial_match();
        });
    }


    private void bind_emoji_shortcut() {
#if EMOJI_DICT
        string[] accelerators = m_settings_emoji.get_strv("hotkey");

        var keybinding_manager = KeybindingManager.get_instance();

        foreach (var accelerator in accelerators) {
            BindingCommon.keybinding_manager_bind(
                    keybinding_manager,
                    ref m_keybindings,
                    accelerator,
                    BindingCommon.KeyEventFuncType.EMOJI_TYPING,
                    handle_emoji_typing,
                    null);
        }
#endif
    }


    private void set_emoji_favorites() {
        m_emojier_favorites = m_settings_emoji.get_strv("favorites");
        IBusEmojier.set_favorites(
                m_emojier_favorites,
                m_settings_emoji.get_strv("favorite-annotations"));
    }


    private void set_emoji_lang() {
        if (m_emojier_set_emoji_lang_id > 0) {
            GLib.Source.remove(m_emojier_set_emoji_lang_id);
            m_emojier_set_emoji_lang_id = 0;
        }
        m_emojier_set_emoji_lang_id = GLib.Idle.add(() => {
            IBusEmojier.set_annotation_lang(
                    m_settings_emoji.get_string("lang"));
            m_emojier_set_emoji_lang_id = 0;
            IBusEmojier.load_unicode_dict();
            return false;
        });
    }


    private void set_emoji_partial_match() {
        IBusEmojier.set_partial_match(
                m_settings_emoji.get_boolean("has-partial-match"));
        IBusEmojier.set_partial_match_length(
                m_settings_emoji.get_int("partial-match-length"));
        IBusEmojier.set_partial_match_condition(
                m_settings_emoji.get_int("partial-match-condition"));
    }


    public void load_settings() {
        BindingCommon.unbind_switch_shortcut(BindingCommon.KeyEventFuncType.ANY,
                                             m_keybindings);
        bind_emoji_shortcut();
        BindingCommon.set_custom_font(m_settings_panel,
                                      m_settings_emoji,
                                      ref m_css_provider);
        set_emoji_favorites();
        set_emoji_lang();
        set_emoji_partial_match();
    }


    /**
     * disconnect_signals:
     *
     * Call this API before m_panel = null so that the ref_count becomes 0
     */
    public void disconnect_signals() {
        if (m_emojier_set_emoji_lang_id > 0) {
            GLib.Source.remove(m_emojier_set_emoji_lang_id);
            m_emojier_set_emoji_lang_id = 0;
        }
    }


    private void show_emojier(Gdk.Event event) {
        m_emojier = new IBusEmojier();
        string emoji = m_emojier.run(m_real_current_context_path, event);
        if (emoji == null) {
            m_emojier = null;
            return;
        }
        this.emojier_focus_commit();
    }


    private void handle_emoji_typing(Gdk.Event event) {
        if (m_emojier != null && m_emojier.is_running()) {
            m_emojier.present_centralize(event);
            return;
        }
        show_emojier(event);
    }


    private bool emojier_focus_commit_real() {
        if (m_emojier == null)
            return true;
        string selected_string = m_emojier.get_selected_string();
        string prev_context_path = m_emojier.get_input_context_path();
        if (selected_string != null &&
            prev_context_path != "" &&
            prev_context_path == m_current_context_path) {
            IBus.Text text = new IBus.Text.from_string(selected_string);
            commit_text(text);
            m_emojier = null;
            bool has_favorite = false;
            foreach (unowned string favorite in m_emojier_favorites) {
                if (favorite == selected_string) {
                    has_favorite = true;
                    break;
                }
            }
            if (!has_favorite) {
                m_emojier_favorites += selected_string;
                m_settings_emoji.set_strv("favorites", m_emojier_favorites);
            }
            return true;
        }

        return false;
    }


    private void emojier_focus_commit() {
        if (m_emojier == null)
            return;
        string selected_string = m_emojier.get_selected_string();
        string prev_context_path = m_emojier.get_input_context_path();
        if (selected_string == null &&
            prev_context_path != "" &&
            m_emojier.is_running()) {
            var context = GLib.MainContext.default();
            if (m_emojier_focus_commit_text_id > 0 &&
                context.find_source_by_id(m_emojier_focus_commit_text_id)
                        != null) {
                GLib.Source.remove(m_emojier_focus_commit_text_id);
            }
            m_emojier_focus_commit_text_id = GLib.Timeout.add(100, () => {
                // focus_in is comming before switcher returns
                emojier_focus_commit_real();
                m_emojier_focus_commit_text_id = -1;
                return false;
            });
        } else {
            if (emojier_focus_commit_real()) {
                var context = GLib.MainContext.default();
                if (m_emojier_focus_commit_text_id > 0 &&
                    context.find_source_by_id(m_emojier_focus_commit_text_id)
                            != null) {
                    GLib.Source.remove(m_emojier_focus_commit_text_id);
                }
                m_emojier_focus_commit_text_id = -1;
            }
        }
    }


    public override void focus_in(string input_context_path) {
        m_current_context_path = input_context_path;

        /* 'fake' input context is named as 
         * '/org/freedesktop/IBus/InputContext_1' and always send in
         * focus-out events by ibus-daemon for the global engine mode.
         * Now ibus-daemon assumes to always use the global engine.
         * But this event should not be used for modal dialogs
         * such as Switcher.
         */
        if (!input_context_path.has_suffix("InputContext_1")) {
            m_real_current_context_path = m_current_context_path;
            this.emojier_focus_commit();
        }
    }


    public override void focus_out(string input_context_path) {
        m_current_context_path = "";
    }


    public override void panel_extension_received(GLib.Variant data) {
        IBus.XEvent? xevent = IBus.Serializable.deserialize_object(data)
                as IBus.XEvent;
        if (xevent == null) {
            warning ("Failed to deserialize IBusXEvent");
            return;
        }
        if (xevent.get_purpose() != "emoji") {
            string format = "The purpose %s is not implemented in PanelExtension";
            warning (format.printf(xevent.get_purpose()));
            return;
        }
        Gdk.EventType event_type;
        if (xevent.get_event_type() == IBus.XEventType.KEY_PRESS) {
            event_type = Gdk.EventType.KEY_PRESS;
        } else if (xevent.get_event_type() == IBus.XEventType.KEY_RELEASE) {
            event_type = Gdk.EventType.KEY_RELEASE;
        } else {
            warning ("Not supported type %d".printf(xevent.get_event_type()));
            return;
        }
        Gdk.Event event = new Gdk.Event(event_type);
        event.key.time = xevent.get_time();
        Gdk.Display? display = Gdk.Display.get_default();
        X.Window xid = xevent.get_window();
        Gdk.X11.Window window;
        window = Gdk.X11.Window.lookup_for_display(
                display as Gdk.X11.Display, xid);
        if (window != null) {
            event.key.window = window;
        } else {
            window = new Gdk.X11.Window.foreign_for_display(
                    display as Gdk.X11.Display, xid);
            event.key.window = window;
        }
        handle_emoji_typing(event);
    }
}
