/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright (c) 2017 Takao Fujiwara <takao.fujiwara1@gmail.com>
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

class Emojier : Gtk.Window {
    private class EEntry : Gtk.SearchEntry {
        public EEntry() {
            GLib.Object(
                name : "IBusEmojierEntry",
                margin_start : 6,
                margin_end : 6,
                margin_top : 6,
                margin_bottom : 6
            );
        }
    }
    private class EListBox : Gtk.ListBox {
        public EListBox() {
            GLib.Object(
                vexpand : true,
                halign : Gtk.Align.FILL,
                valign : Gtk.Align.FILL
            );
        }
    }
    private class EBoxRow : Gtk.ListBoxRow {
        public EBoxRow(string text,
                       string id="") {
            this.text = text;
            this.id = id;
        }

        public string text { get; set; }
        public string id { get; set; }
    }
    private class EScrolledWindow : Gtk.ScrolledWindow {
        public EScrolledWindow(Gtk.Adjustment? hadjustment=null,
                               Gtk.Adjustment? vadjustment=null) {
            GLib.Object(
                hscrollbar_policy : Gtk.PolicyType.NEVER,
                vscrollbar_policy : Gtk.PolicyType.NEVER,
                shadow_type : Gtk.ShadowType.IN,
                margin_start : 6,
                margin_end : 6,
                margin_top : 6,
                margin_bottom : 6
            );
            if (hadjustment != null)
                set_hadjustment(hadjustment);
            if (vadjustment != null)
                set_hadjustment(vadjustment);
        }
    }
    private class EGrid : Gtk.Grid {
        public EGrid() {
            GLib.Object(
                vexpand : true,
                halign : Gtk.Align.FILL,
                valign : Gtk.Align.FILL
            );
        }
    }
    private class EPaddedLabel : Gtk.Box {
        public EPaddedLabel(string          text,
                            Gtk.Align       align,
                            TravelDirection direction=TravelDirection.NONE) {
            GLib.Object(
                name : "IBusEmojierPaddedLabel",
                orientation : Gtk.Orientation.HORIZONTAL,
                spacing : 0
            );
            if (direction == TravelDirection.BACKWARD) {
                IconWidget icon;
                if (Gtk.Widget.get_default_direction() ==
                    Gtk.TextDirection.RTL) {
                    icon = new IconWidget("go-previous-rtl-symbolic",
                                          Gtk.IconSize.MENU);
                } else {
                    icon = new IconWidget("go-previous-symbolic",
                                          Gtk.IconSize.MENU);
                }
                pack_start(icon, false, true, 0);
            }
            Gtk.Label label = new Gtk.Label(text);
            label.set_halign(align);
            label.set_valign(Gtk.Align.CENTER);
            label.set_margin_start(20);
            label.set_margin_end(20);
            label.set_margin_top(6);
            label.set_margin_bottom(6);
            pack_start(label, true, true, 0);
        }
    }
    private class ETitleLabel : Gtk.Box {
        private Gtk.Button m_close_button;
        private ulong m_close_handler;

        public ETitleLabel(string    text,
                           Gtk.Align align) {
            GLib.Object(
                name : "IBusEmojierTitleLabel",
                orientation : Gtk.Orientation.HORIZONTAL,
                spacing : 0
            );
            Gtk.Label label = new Gtk.Label(text);
            label.set_halign(align);
            label.set_valign(align);
            label.set_margin_start(20);
            label.set_margin_end(20);
            label.set_margin_top(6);
            label.set_margin_bottom(6);
            pack_start(label, true, true, 0);
            IconWidget icon = new IconWidget("window-close", Gtk.IconSize.MENU);
            m_close_button = new Gtk.Button();
            m_close_button.add(icon);
            pack_end(m_close_button, false, true, 0);
        }
        public void set_loop(GLib.MainLoop? loop) {
            if (m_close_handler > 0)
                GLib.SignalHandler.disconnect(m_close_button, m_close_handler);
            m_close_handler = m_close_button.button_press_event.connect((e) => {
                if (loop != null && loop.is_running())
                    loop.quit();
                return true;
            });
        }
        public void unset_loop() {
            if (m_close_handler > 0) {
                GLib.SignalHandler.disconnect(m_close_button, m_close_handler);
                m_close_handler = 0;
            }
        }
    }

    private enum TravelDirection {
        NONE,
        BACKWARD,
    }

    private enum CategoryType {
        EMOJI,
        LANG,
    }

    private const uint EMOJI_GRID_PAGE = 10;
    private Gtk.Box m_vbox;
    private ETitleLabel m_title;
    private EEntry m_entry;
    private string? m_backward;
    private EScrolledWindow? m_scrolled_window = null;
    private EListBox m_list_box;
    private CategoryType m_current_category_type = CategoryType.EMOJI;
    private bool m_is_running = false;
    private string m_input_context_path = "";
    private GLib.StringBuilder m_buffer_string;
    private GLib.MainLoop? m_loop;
    private string? m_result;
    private GLib.SList<string> m_lang_list;
    private string m_current_lang = "en";
    private string? m_unicode_point = null;
    private bool m_candidate_panel_is_visible;
    private GLib.HashTable<string, GLib.SList> m_annotation_to_emojis_dict;
    private GLib.HashTable<string, IBus.EmojiData> m_emoji_to_data_dict;
    private GLib.HashTable<string, GLib.SList> m_category_to_emojis_dict;
    int m_category_active_index;
    private int m_emoji_max_seq_len = 0;
    private IBus.LookupTable m_lookup_table;
    private Gtk.Label[] m_candidates;
    private string m_emoji_font = "Monospace 16";
    private string[] m_favorites = {};
    // TODO: Get the selected color from CandidateArea
    private Gdk.RGBA m_selected_fg_color = Gdk.RGBA(){
            red = 1.0, green = 1.0, blue = 1.0, alpha = 1.0 };
    private Gdk.RGBA m_selected_bg_color = Gdk.RGBA(){
            red = 0.300, green = 0.565, blue = 0.851, alpha = 1.0 };

    public signal void candidate_clicked(uint index, uint button, uint state);

    public Emojier() {
        GLib.Object(
            type : Gtk.WindowType.POPUP,
            events : Gdk.EventMask.KEY_PRESS_MASK |
                     Gdk.EventMask.KEY_RELEASE_MASK |
                     Gdk.EventMask.BUTTON_PRESS_MASK,
            window_position : Gtk.WindowPosition.CENTER,
            accept_focus : true,
            decorated : false,
            modal : true,
            resizable : true,
            focus_visible : true
        );

        Gdk.Display display = Gdk.Display.get_default();
        Gdk.Screen screen = (display != null) ?
                display.get_default_screen() : null;

        if (screen == null) {
            warning("Could not open display.");
            return;
        }
        string data = "grid { background-color: #ffffff; }";
        Gtk.CssProvider css_provider = new Gtk.CssProvider();
        try {
            css_provider.load_from_data(data, -1);
        } catch (GLib.Error e) {
            warning("Failed css_provider_from_data: %s", e.message);
            return;
        }

        Gtk.StyleContext.add_provider_for_screen(
                screen,
                css_provider,
                Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);

        m_vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
        add(m_vbox);

        m_title = new ETitleLabel(_("Emoji Dialog"),
                                  Gtk.Align.CENTER);
        m_vbox.add(m_title);
        m_entry = new EEntry();
        m_entry.set_placeholder_text(_("Type annotation or choose emoji"));
        m_vbox.add(m_entry);
        m_entry.changed.connect(() => {
            m_buffer_string.assign(m_entry.get_text());
            update_cadidate_window();
        });
        m_entry.icon_release.connect((icon_pos, event) => {
            m_buffer_string.erase();
            hide_candidate_panel();
        });

        /* Set the accessible role of the label to a status bar so it
         * will emit name changed events that can be used by screen
         * readers.
         */
        Atk.Object obj = m_entry.get_accessible();
        obj.set_role (Atk.Role.STATUSBAR);

        m_buffer_string = new StringBuilder();
        grab_focus();

        // The constructor of IBus.LookupTable does not support more than
        // 16 pages.
        m_lookup_table = new IBus.LookupTable(1, 0, true, true);
        m_lookup_table.set_page_size(EMOJI_GRID_PAGE * EMOJI_GRID_PAGE);

        hide.connect(() => {
            if (m_loop != null && m_loop.is_running())
                m_loop.quit();
        });

        candidate_clicked.connect((i, b, s) => {
            IBus.Text candidate = m_lookup_table.get_candidate(i);
            m_result = candidate.text;
            m_loop.quit();
            hide_candidate_panel();
        });

        GLib.Idle.add(() => {
            m_lang_list = read_lang_list();
            reload_emoji_dict();
            return false;
        });
    }

    private GLib.SList<string> read_lang_list() {
        GLib.SList<string> lang_list = new GLib.SList<string>();
        const string dict_path = Config.PKGDATADIR + "/dicts";
        GLib.Dir dir = null;
        try {
            dir = GLib.Dir.open(dict_path);
        } catch (GLib.FileError e) {
            warning("Error loading %s: %s", dict_path, e.message);
            return lang_list;
        }
        string name;
        while ((name = dir.read_name()) != null) {
            const string dict_suffix = ".dict";
            const string dict_prefix = "emoji-";
            if (name.has_suffix(dict_suffix)) {
                name = name[0:name.length - dict_suffix.length];
                if (name.has_prefix(dict_prefix)) {
                    name = name[dict_prefix.length:name.length];
                    lang_list.append(name);
                } else {
                    warning("Need %s prefix in the filename: %s/%s%s",
                            dict_prefix, dict_path, name, dict_suffix);
                }
            } else {
                warning("Need %s extention in the filename: %s/%s",
                        dict_suffix, dict_path, name);
            }
        }
        lang_list.sort((a, b) => {
            string a_lang = IBus.get_language_name(a);
            string b_lang = IBus.get_language_name(b);
            return GLib.strcmp(a_lang, b_lang);
        });
        return lang_list;
    }

    private void reload_emoji_dict() {
        init_emoji_dict();
        make_emoji_dict("en");
        if (m_current_lang != "en")
            make_emoji_dict(m_current_lang);
    }

    private void init_emoji_dict() {
        m_annotation_to_emojis_dict =
                new GLib.HashTable<string, GLib.SList>(GLib.str_hash,
                                                       GLib.str_equal);
        m_emoji_to_data_dict =
                new GLib.HashTable<string, IBus.EmojiData>(GLib.str_hash,
                                                           GLib.str_equal);
        m_category_to_emojis_dict =
                new GLib.HashTable<string, GLib.SList>(GLib.str_hash,
                                                       GLib.str_equal);
    }

    private void make_emoji_dict(string lang) {
        GLib.SList<IBus.EmojiData> emoji_list = IBus.EmojiData.load(
                    Config.PKGDATADIR + "/dicts/emoji-" + lang + ".dict");
        if (emoji_list == null)
            return;
        foreach (IBus.EmojiData data in emoji_list) {
            update_annotation_to_emojis_dict(data);
            update_emoji_to_data_dict(data, lang);
            update_category_to_emojis_dict(data, lang);
        }
        GLib.List<unowned string> annotations =
                m_annotation_to_emojis_dict.get_keys();
        foreach (unowned string annotation in annotations) {
            if (m_emoji_max_seq_len < annotation.length)
                m_emoji_max_seq_len = annotation.length;
        }
    }

    private void update_annotation_to_emojis_dict (IBus.EmojiData data) {
        string emoji = data.get_emoji();
        unowned GLib.SList<string> annotations = data.get_annotations();
        foreach (string annotation in annotations) {
            bool has_emoji = false;
            unowned GLib.SList<string> hits =
                    m_annotation_to_emojis_dict.lookup(annotation);
            foreach (string hit_emoji in hits) {
                if (hit_emoji == emoji) {
                    has_emoji = true;
                    break;
                }
            }
            if (!has_emoji) {
                hits.append(emoji);
                m_annotation_to_emojis_dict.replace(annotation, hits.copy());
            }
        }
    }

    private void update_emoji_to_data_dict (IBus.EmojiData data,
                                            string         lang) {
        string emoji = data.get_emoji();
        if (lang == "en") {
            m_emoji_to_data_dict.replace(emoji, data);
        } else {
            unowned IBus.EmojiData? en_data =
                    m_emoji_to_data_dict.lookup(emoji);
            if (en_data == null) {
                warning("No IBusEmojiData for English: %s".printf(emoji));
                m_emoji_to_data_dict.insert(emoji, data);
                return;
            }
            unowned GLib.SList<string> annotations = data.get_annotations();
            unowned GLib.SList<string> en_annotations
                = en_data.get_annotations();
            foreach (string annotation in en_annotations) {
                if (annotations.find_custom(annotation, GLib.strcmp) == null)
                    annotations.append(annotation.dup());
            }
            en_data.set_annotations(annotations.copy_deep(GLib.strdup));
        }
    }

    private void update_category_to_emojis_dict (IBus.EmojiData data,
                                                 string         lang) {
        string emoji = data.get_emoji();
        string category = data.get_category();
        if (lang == "en" && category != "") {
            bool has_emoji = false;
            unowned GLib.SList<string> hits =
                    m_category_to_emojis_dict.lookup(category);
            foreach (string hit_emoji in hits) {
                if (hit_emoji == emoji) {
                    has_emoji = true;
                    break;
                }
            }
            if (!has_emoji) {
                hits.append(emoji);
                m_category_to_emojis_dict.replace(category, hits.copy());
            }
        }
    }

    private void set_fixed_size() {
        if (!m_candidate_panel_is_visible &&
            m_current_category_type == CategoryType.LANG) {
            Gtk.PolicyType vpolicy;
            m_scrolled_window.get_policy(null, out vpolicy);
            if (vpolicy == Gtk.PolicyType.AUTOMATIC)
                return;
            int width, height;
            get_size(out width, out height);
            set_size_request(width, height);
            if (m_scrolled_window != null) {
                m_scrolled_window.set_policy(Gtk.PolicyType.NEVER,
                                             Gtk.PolicyType.AUTOMATIC);
            }
        } else {
            resize(20, 1);
            if (m_scrolled_window != null) {
                m_scrolled_window.set_policy(Gtk.PolicyType.NEVER,
                                             Gtk.PolicyType.NEVER);
            }
        }
    }

    private void remove_all_children() {
        foreach (Gtk.Widget w in m_vbox.get_children()) {
            if (w.name == "IBusEmojierEntry" ||
                w.name == "IBusEmojierTitleLabel") {
                continue;
            }
            w.destroy();
        }
    }

    private void show_category_list() {
        remove_all_children();
        m_scrolled_window = new EScrolledWindow();
        set_fixed_size();
        string language = IBus.get_language_name(m_current_lang);
        EPaddedLabel label = new EPaddedLabel(language, Gtk.Align.CENTER);
        Gtk.Button button = new Gtk.Button();
        button.add(label);
        m_vbox.add(button);
        button.show_all();
        button.button_press_event.connect((e) => {
            m_category_active_index = 0;
            m_current_category_type = CategoryType.LANG;
            show_category_list();
            return true;
        });

        m_vbox.add(m_scrolled_window);
        Gtk.Viewport viewport = new Gtk.Viewport(null, null);
        m_scrolled_window.add(viewport);

        m_list_box = new EListBox();
        viewport.add(m_list_box);
        Gtk.Adjustment adjustment = m_scrolled_window.get_vadjustment();
        m_list_box.set_adjustment(adjustment);
        if (m_current_category_type == CategoryType.EMOJI) {
            m_list_box.row_activated.connect((box, gtkrow) => {
                m_category_active_index = 0;
                EBoxRow row = gtkrow as EBoxRow;
                show_emoji_for_category(row);
            });

            uint n = 1;
            if (m_favorites.length > 0) {
                EBoxRow row = new EBoxRow("@favorites");
                EPaddedLabel widget =
                        new EPaddedLabel(_("Favorites"), Gtk.Align.CENTER);
                row.add(widget);
                m_list_box.add(row);
                if (n++ == m_category_active_index)
                    m_list_box.select_row(row);
            }
            GLib.List<unowned string> categories =
                    m_category_to_emojis_dict.get_keys();
            foreach (unowned string category in categories) {
                EBoxRow row = new EBoxRow(category);
                string locale_category = _(category);
                StringBuilder capital_category = new StringBuilder();
                for (int i = 0; i < locale_category.char_count(); i++) {
                    unichar ch = locale_category.get_char(i);
                    if (i == 0)
                        capital_category.append_unichar(ch.toupper());
                    else
                        capital_category.append_unichar(ch);
                }
                EPaddedLabel widget =
                        new EPaddedLabel(capital_category.str,
                                         Gtk.Align.CENTER);
                row.add(widget);
                m_list_box.add(row);
                if (n++ == m_category_active_index)
                    m_list_box.select_row(row);
            }
        } else if (m_current_category_type == CategoryType.LANG) {
            m_list_box.row_activated.connect((box, gtkrow) => {
                m_category_active_index = 0;
                EBoxRow row = gtkrow as EBoxRow;
                if (m_current_lang != row.id) {
                    m_current_lang = row.id;
                    reload_emoji_dict();
                }
                m_current_category_type = CategoryType.EMOJI;
                show_category_list();
            });
            uint n = 1;
            foreach (unowned string id in m_lang_list) {
                string selected_language = IBus.get_language_name(id);
                EBoxRow row = new EBoxRow("", id);
                EPaddedLabel widget =
                        new EPaddedLabel(selected_language, Gtk.Align.CENTER);
                row.add(widget);
                m_list_box.add(row);
                if (n++ == m_category_active_index)
                    m_list_box.select_row(row);
            }
        }

        m_scrolled_window.show_all();
        if (m_category_active_index == 0)
            m_list_box.unselect_all();
        m_list_box.invalidate_filter();
        m_list_box.set_selection_mode(Gtk.SelectionMode.SINGLE);
    }

    private void show_emoji_for_category(EBoxRow row) {
        if (row.text == "@favorites") {
            m_lookup_table.clear();
            foreach (unowned string favorate in m_favorites) {
                IBus.Text text = new IBus.Text.from_string(favorate);
                m_lookup_table.append_candidate(text);
            }
            m_backward = _("Favorites");
        } else {
            unowned GLib.SList<unowned string> emojis =
                    m_category_to_emojis_dict.lookup(row.text);
            m_lookup_table.clear();
            foreach (unowned string emoji in emojis) {
                IBus.Text text = new IBus.Text.from_string(emoji);
                m_lookup_table.append_candidate(text);
            }
            m_backward = row.text;
        }
        show_candidate_panel();
    }

    private void label_set_active_color(Gtk.Label label) {
        unowned string text = label.get_text();
        Pango.AttrList attrs = new Pango.AttrList();
        Pango.Attribute pango_attr = Pango.attr_foreground_new(
                (uint16)(m_selected_fg_color.red * uint16.MAX),
                (uint16)(m_selected_fg_color.green * uint16.MAX),
                (uint16)(m_selected_fg_color.blue * uint16.MAX));
        pango_attr.start_index = 0;
        pango_attr.end_index = text.char_count();
        attrs.insert((owned)pango_attr);

        pango_attr = Pango.attr_background_new(
                (uint16)(m_selected_bg_color.red * uint16.MAX),
                (uint16)(m_selected_bg_color.green * uint16.MAX),
                (uint16)(m_selected_bg_color.blue * uint16.MAX));
        pango_attr.start_index = 0;
        pango_attr.end_index = text.char_count();
        attrs.insert((owned)pango_attr);
        label.set_attributes(attrs);
    }

    private void show_arrow_buttons() {
        Gtk.Button next_button = new Gtk.Button();
        next_button.clicked.connect(() => {
            m_lookup_table.page_down();
            show_candidate_panel();
        });
        next_button.set_image(new Gtk.Image.from_icon_name(
                              "go-down",
                              Gtk.IconSize.MENU));
        next_button.set_relief(Gtk.ReliefStyle.NONE);
        next_button.set_tooltip_text(_("Page Down"));

        Gtk.Button prev_button = new Gtk.Button();
        prev_button.clicked.connect(() => {
            m_lookup_table.page_up();
            show_candidate_panel();
        });
        prev_button.set_image(new Gtk.Image.from_icon_name(
                              "go-up",
                              Gtk.IconSize.MENU));
        prev_button.set_relief(Gtk.ReliefStyle.NONE);
        prev_button.set_tooltip_text(_("Page Up"));

        Gtk.Box buttons_hbox = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
        Gtk.Label state_label = new Gtk.Label(null);
        state_label.set_size_request(10, -1);
        state_label.set_halign(Gtk.Align.CENTER);
        state_label.set_valign(Gtk.Align.CENTER);
        buttons_hbox.pack_start(state_label, false, true, 0);
        buttons_hbox.pack_start(prev_button, false, false, 0);
        buttons_hbox.pack_start(next_button, false, false, 0);
        m_vbox.pack_start(buttons_hbox, false, false, 0);
        buttons_hbox.show_all();
    }

    private bool check_unicode_point(bool check_xdigit_only) {
        m_unicode_point = null;
        GLib.StringBuilder buff = new GLib.StringBuilder();
        for (int i = 0; i < m_buffer_string.str.char_count(); i++) {
            unichar ch = m_buffer_string.str.get_char(i);
            if (ch == 0)
                return false;
            if (!ch.isxdigit())
                return false;
            buff.append_unichar(ch);
        }
        unichar code = (unichar)buff.str.to_ulong(null, 16);
        if (!code.validate())
            return false;
        if (check_xdigit_only)
            return true;
        m_unicode_point = code.to_string();
        if (m_unicode_point == null)
            return true;
        IBus.Text text = new IBus.Text.from_string(m_unicode_point);
        m_lookup_table.append_candidate(text);
        return true;
    }

    public void update_cadidate_window() {
        string annotation = m_entry.get_text();
        if (annotation.length == 0) {
            hide_candidate_panel();
            m_backward = null;
            return;
        }
        if (annotation.length > m_emoji_max_seq_len) {
            hide_candidate_panel();
            return;
        }
        // Call check_unicode_point() to get m_unicode_point
        check_unicode_point(false);
        unowned GLib.SList<string>? emojis =
            m_annotation_to_emojis_dict.lookup(annotation);
        if (emojis == null && m_unicode_point == null) {
            hide_candidate_panel();
            return;
        }
        m_lookup_table.clear();
        // Call check_unicode_point() to update m_lookup_table
        check_unicode_point(false);
        foreach (unowned string emoji in emojis) {
            IBus.Text text = new IBus.Text.from_string(emoji);
            m_lookup_table.append_candidate(text);
        }
        show_candidate_panel();
    }

    private void show_candidate_panel() {
        remove_all_children();
        set_fixed_size();
        uint page_size = m_lookup_table.get_page_size();
        uint ncandidates = m_lookup_table.get_number_of_candidates();
        uint cursor = m_lookup_table.get_cursor_pos();

        uint page_start_pos = cursor / page_size * page_size;
        uint page_end_pos = uint.min(page_start_pos + page_size, ncandidates);
        if (m_backward != null) {
            string backward_desc =
                    "%s (%u / %u)".printf(m_backward,
                                          cursor / page_size + 1,
                                          ncandidates / page_size + 1);
            EPaddedLabel label = new EPaddedLabel(backward_desc,
                                                  Gtk.Align.CENTER,
                                                  TravelDirection.BACKWARD);
            Gtk.Button button = new Gtk.Button();
            button.add(label);
            m_vbox.add(button);
            button.show_all();
            button.button_press_event.connect((w, e) => {
                hide_candidate_panel();
                return true;
            });
        }
        EGrid grid = new EGrid();
        int n = 0;
        for (uint i = page_start_pos; i < page_end_pos; i++) {
            IBus.Text candidate = m_lookup_table.get_candidate(i);
            Gtk.Label label = new Gtk.Label(candidate.text);
            string emoji_font = m_emoji_font;
            if (candidate.text.char_count() > 2) {
                Pango.FontDescription font_desc =
                        Pango.FontDescription.from_string(emoji_font);
                string font_family = font_desc.get_family();
                int font_size = font_desc.get_size() / Pango.SCALE;
                emoji_font = "%s %d".printf(font_family, font_size -2);
            }
            string markup = "<span font=\"%s\">%s</span>".
                    printf(emoji_font, candidate.get_text());
            label.set_markup(markup);
            label.set_halign(Gtk.Align.FILL);
            label.set_valign(Gtk.Align.FILL);
            if (i == cursor) {
                label_set_active_color(label);
            }
            Gtk.EventBox candidate_ebox = new Gtk.EventBox();
            candidate_ebox.add(label);
            // Make a copy of i to workaround a bug in vala.
            // https://bugzilla.gnome.org/show_bug.cgi?id=628336
            uint index = i;
            candidate_ebox.button_press_event.connect((w, e) => {
                candidate_clicked(index, e.button, e.state);
                return true;
            });
            grid.attach(candidate_ebox,
                        n % (int)EMOJI_GRID_PAGE, n / (int)EMOJI_GRID_PAGE,
                        1, 1);
            n++;

            m_candidates += label;
        }
        if (n > 0) {
            m_candidate_panel_is_visible = true;
            show_arrow_buttons();
            m_vbox.add(grid);
            grid.show_all();
            IBus.Text candidate = m_lookup_table.get_candidate(cursor);
            if (cursor == 0 && candidate.text == m_unicode_point) {
                EPaddedLabel widget = new EPaddedLabel(
                        _("Description: Unicode point U+%04X").printf(
                                m_unicode_point.get_char(0)),
                        Gtk.Align.START);
                m_vbox.add(widget);
                widget.show_all();
                return;
            }
            unowned IBus.EmojiData data =
                    m_emoji_to_data_dict.lookup(candidate.text);
            unowned string description = data.get_description();
            if (description != "") {
                EPaddedLabel widget = new EPaddedLabel(
                        _("Description: %s").printf(description),
                        Gtk.Align.START);
                m_vbox.add(widget);
                widget.show_all();
            }
            unowned GLib.SList<unowned string>? annotations =
                    data.get_annotations();
            GLib.StringBuilder buff = new GLib.StringBuilder();
            int i = 0;
            foreach (unowned string annotation in annotations) {
                if (i++ == 0)
                    buff.append_printf(_("Annotations: %s"), annotation);
                else
                    buff.append_printf(" | %s", annotation);
                if (buff.str.char_count() > 30) {
                    EPaddedLabel widget = new EPaddedLabel(buff.str,
                                                           Gtk.Align.START);
                    m_vbox.add(widget);
                    widget.show_all();
                    buff.erase();
                }
            }
            if (buff.str != "") {
                EPaddedLabel widget = new EPaddedLabel(buff.str,
                                                       Gtk.Align.START);
                m_vbox.add(widget);
                widget.show_all();
            }
        }
    }

    private void hide_candidate_panel() {
        m_candidate_panel_is_visible = false;
        if (m_loop.is_running())
            show_category_list();
    }

    private bool if_in_range_of_lookup(uint keyval) {
        if (!m_candidate_panel_is_visible)
            return false;
        string backup_annotation = m_buffer_string.str.dup();
        unichar ch = IBus.keyval_to_unicode (keyval);
        m_buffer_string.append_unichar(ch);
        if (check_unicode_point(true)) {
            m_buffer_string.assign(backup_annotation);
            return false;
        }
        m_buffer_string.assign(backup_annotation);
        if (keyval < Gdk.Key.@0 || keyval > Gdk.Key.@9)
            return false;
        if (keyval == Gdk.Key.@0)
            keyval = Gdk.Key.@9 + 1;
        uint index = keyval - Gdk.Key.@1 + 1;
        uint candidates = m_lookup_table.get_number_of_candidates();
        uint cursor_pos = m_lookup_table.get_cursor_pos();
        uint page_size = m_lookup_table.get_page_size();
        if (index > uint.min(candidates - (cursor_pos / page_size) * page_size,
                             page_size)) {
            return false;
        }
        return true;
    }

    private void set_number_on_lookup(uint keyval) {
        if (keyval == Gdk.Key.@0)
            keyval = Gdk.Key.@9 + 1;
        uint index = keyval - Gdk.Key.@1;
        uint cursor_pos = m_lookup_table.get_cursor_pos();
        uint cursor_in_page= m_lookup_table.get_cursor_in_page();
        uint real_index = cursor_pos - cursor_in_page + index;
        m_lookup_table.set_cursor_pos(real_index);
        IBus.Text text = m_lookup_table.get_candidate(real_index);
        m_result = text.text;
    }

    private void candidate_panel_cursor_down() {
        uint ncandidates = m_lookup_table.get_number_of_candidates();
        uint cursor = m_lookup_table.get_cursor_pos();
        if ((cursor + EMOJI_GRID_PAGE) < ncandidates) {
            m_lookup_table.set_cursor_pos(cursor + EMOJI_GRID_PAGE);
        } else if (cursor % EMOJI_GRID_PAGE < ncandidates) {
                m_lookup_table.set_cursor_pos(cursor % EMOJI_GRID_PAGE);
        } else {
            m_lookup_table.set_cursor_pos(0);
        }
        show_candidate_panel();
    }

    private void candidate_panel_cursor_up() {
        int ncandidates = (int)m_lookup_table.get_number_of_candidates();
        int cursor = (int)m_lookup_table.get_cursor_pos();
        int highest_pos =
            (ncandidates / (int)EMOJI_GRID_PAGE * (int)EMOJI_GRID_PAGE)
            + (cursor % (int)EMOJI_GRID_PAGE);
        if ((cursor - (int)EMOJI_GRID_PAGE) >= 0) {
            m_lookup_table.set_cursor_pos(cursor - (int)EMOJI_GRID_PAGE);
        } else if (highest_pos < ncandidates) {
            m_lookup_table.set_cursor_pos(highest_pos);
        } else {
            m_lookup_table.set_cursor_pos(0);
        }
        show_candidate_panel();
    }

    private void category_list_cursor_move(uint keyval) {
        GLib.List<weak Gtk.Widget> list = m_list_box.get_children();
        if (keyval == Gdk.Key.Down) {
            m_category_active_index =
                    ++m_category_active_index % ((int)list.length() + 1);
        } else if (keyval == Gdk.Key.Up) {
            if (--m_category_active_index < 0)
                    m_category_active_index = (int)list.length();
        }
        Gtk.Adjustment adjustment = m_list_box.get_adjustment();
        m_scrolled_window.set_vadjustment(adjustment);
        show_category_list();
    }

    public string run(Gdk.Event event,
                      string    input_context_path) {
        assert (m_loop == null);

        m_is_running = true;
        m_input_context_path = input_context_path;
        m_candidate_panel_is_visible = false;
        m_result = null;

        /* Let gtk recalculate the window size. */
        resize(1, 1);

        m_entry.set_text("");
        m_buffer_string.erase();

        Gdk.Device device = event.get_device();
        if (device == null) {
            var display = get_display();
            var device_manager = display.get_device_manager();
            device = device_manager.list_devices(Gdk.DeviceType.MASTER).data;
        }

        Gdk.Device keyboard;
        Gdk.Device pointer;
        if (device.get_source() == Gdk.InputSource.KEYBOARD) {
            keyboard = device;
            pointer = device.get_associated_device();
        } else {
            pointer = device;
            keyboard = device.get_associated_device();
        }

        m_current_category_type = CategoryType.EMOJI;
        show_category_list();
        m_entry.set_activates_default(true);
        show_all();

        Gdk.GrabStatus status;
        // Grab all keyboard events
        status = keyboard.grab(get_window(),
                               Gdk.GrabOwnership.NONE,
                               true,
                               Gdk.EventMask.KEY_PRESS_MASK |
                               Gdk.EventMask.KEY_RELEASE_MASK,
                               null,
                               Gdk.CURRENT_TIME);
        if (status != Gdk.GrabStatus.SUCCESS)
            warning("Grab keyboard failed! status = %d", status);
        // Grab all pointer events
        status = pointer.grab(get_window(),
                              Gdk.GrabOwnership.NONE,
                              true,
                              Gdk.EventMask.BUTTON_PRESS_MASK |
                              Gdk.EventMask.BUTTON_RELEASE_MASK,
                              null,
                              Gdk.CURRENT_TIME);
        if (status != Gdk.GrabStatus.SUCCESS)
            warning("Grab pointer failed! status = %d", status);

        m_loop = new GLib.MainLoop();
        m_title.set_loop(m_loop);
        m_loop.run();
        m_title.unset_loop();
        m_loop = null;

        keyboard.ungrab(Gdk.CURRENT_TIME);
        pointer.ungrab(Gdk.CURRENT_TIME);

        // Need focus-out on Gtk.Entry to send the emoji to applications.
        Gdk.Event fevent = new Gdk.Event(Gdk.EventType.FOCUS_CHANGE);
        fevent.focus_change.in = 0;
        fevent.focus_change.window  = get_window();
        m_entry.send_focus_change(fevent);

        hide();
        // Make sure the switcher is hidden before returning from this function.
        while (Gtk.events_pending())
            Gtk.main_iteration ();
        m_is_running = false;

        return m_result;
    }

    /* override virtual functions */
    public override void show() {
        base.show();
        set_focus_visible(true);
    }

    public override bool key_press_event(Gdk.EventKey event) {
        uint keyval = event.keyval;
        uint modifiers = event.state;

        /* Need to handle events in key_press_event() instead of
         * key_release_event() so that this can know if the event
         * was handled by IME.
         */
        if (if_in_range_of_lookup(keyval)) {
            set_number_on_lookup(keyval);
            m_loop.quit();
            return true;
        }
        switch (keyval) {
        case Gdk.Key.Escape:
            if (m_candidate_panel_is_visible) {
                hide_candidate_panel();
                return true;
            } else if (m_current_category_type == CategoryType.LANG) {
                m_current_category_type = CategoryType.EMOJI;
                show_candidate_panel();
                return true;
            } else if (m_buffer_string.str.length == 0) {
                m_loop.quit();
                hide_candidate_panel();
                return true;
            }
            m_buffer_string.erase();
            break;
        case Gdk.Key.Return:
            if (m_candidate_panel_is_visible) {
                uint index = m_lookup_table.get_cursor_pos();
                IBus.Text text = m_lookup_table.get_candidate(index);
                m_result = text.text;
                m_loop.quit();
                hide_candidate_panel();
            } else if (m_category_active_index > 0) {
                Gtk.ListBoxRow gtkrow = m_list_box.get_selected_row();
                EBoxRow row = gtkrow as EBoxRow;
                show_emoji_for_category(row);
            }
            return true;
        case Gdk.Key.BackSpace:
            if (m_buffer_string.len > 0)
                m_buffer_string.erase(m_buffer_string.len - 1);
            break;
        case Gdk.Key.space:
        case Gdk.Key.KP_Space:
            if ((modifiers & Gdk.ModifierType.SHIFT_MASK) != 0) {
                unichar ch = IBus.keyval_to_unicode (keyval);
                m_buffer_string.append_unichar(ch);
                break;
            }
            if (m_candidate_panel_is_visible) {
                m_lookup_table.cursor_down();
                show_candidate_panel();
            }
            else {
                category_list_cursor_move(Gdk.Key.Down);
            }
            return true;
        case Gdk.Key.Right:
            if (m_candidate_panel_is_visible) {
                m_lookup_table.cursor_down();
                show_candidate_panel();
                return true;
            }
            break;
        case Gdk.Key.Left:
            if (m_candidate_panel_is_visible) {
                m_lookup_table.cursor_up();
                show_candidate_panel();
                return true;
            }
            break;
        case Gdk.Key.Down:
            if (m_candidate_panel_is_visible)
                candidate_panel_cursor_down();
            else
                category_list_cursor_move(Gdk.Key.Down);
            return true;
        case Gdk.Key.Up:
            if (m_candidate_panel_is_visible)
                candidate_panel_cursor_up();
            else
                category_list_cursor_move(Gdk.Key.Up);
            return true;
        case Gdk.Key.Page_Down:
            if (m_candidate_panel_is_visible) {
                m_lookup_table.page_down();
                show_candidate_panel();
                return true;
            }
            break;
        case Gdk.Key.Page_Up:
            if (m_candidate_panel_is_visible) {
                m_lookup_table.page_up();
                show_candidate_panel();
                return true;
            }
            break;
        default:
            unichar ch = IBus.keyval_to_unicode (keyval);
            if (!ch.isgraph())
                return true;
            m_buffer_string.append_unichar(ch);
            break;
        }

        string annotation = m_buffer_string.str;
        m_entry.set_text(annotation);

        return true;
    }

    public bool is_running() {
        return m_is_running;
    }

    public string get_input_context_path() {
        return m_input_context_path;
    }

    public string get_selected_string() {
        return m_result;
    }

    public void reset() {
        m_input_context_path = "";
        m_result = null;
    }

    public void set_emoji_font(string emoji_font) {
        m_emoji_font = emoji_font;
    }

    public void set_favorites (string[]? unowned_favorites) {
        m_favorites = {};
        foreach (string favorite in unowned_favorites) {
            m_favorites += favorite;
        }
    }
}
