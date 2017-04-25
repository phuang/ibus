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

class IBusEmojier : Gtk.Window {
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
        public EBoxRow(string text) {
            this.text = text;
        }

        public string text { get; set; }
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
                row_homogeneous : false,
                vexpand : true,
                halign : Gtk.Align.FILL,
                valign : Gtk.Align.FILL,
                row_spacing : 5,
                column_spacing : 5,
                border_width : 2
            );
        }
    }
    private class EWhiteLabel : Gtk.Label {
        public EWhiteLabel(string text) {
            GLib.Object(
                name : "IBusEmojierWhiteLabel"
            );
            if (text != "")
                set_label(text);
        }
    }
    private class ESelectedLabel : Gtk.Label {
        public ESelectedLabel(string text) {
            GLib.Object(
                name : "IBusEmojierSelectedLabel"
            );
            if (text != "")
                set_label(text);
        }
    }
    private class EPaddedLabel : Gtk.Label {
        public EPaddedLabel(string          text,
                            Gtk.Align       align) {
            GLib.Object(
                name : "IBusEmojierPaddedLabel",
                halign : align,
                valign : Gtk.Align.CENTER,
                margin_start : 20,
                margin_end : 20,
                margin_top : 6,
                margin_bottom : 6
            );
            set_text(text);
        }
    }
    private class EPaddedLabelBox : Gtk.Box {
        public EPaddedLabelBox(string          text,
                               Gtk.Align       align,
                               TravelDirection direction=TravelDirection.NONE) {
            GLib.Object(
                name : "IBusEmojierPaddedLabelBox",
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
            EPaddedLabel label = new EPaddedLabel(text, align);
            pack_start(label, true, true, 0);
        }
    }
    private class ETitleLabelBox : Gtk.HeaderBar {
        private Gtk.Label m_lang_label;

        public ETitleLabelBox(string title) {
            GLib.Object(
                name : "IBusEmojierTitleLabelBox",
                show_close_button: true,
                decoration_layout: ":close",
                title: title
            );
            var vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
            set_custom_title(vbox);
            var label = new Gtk.Label(title);
            label.get_style_context().add_class("title");
            vbox.pack_start(label, true, false, 0);
            m_lang_label = new Gtk.Label(null);
            m_lang_label.get_style_context().add_class("subtitle");
            vbox.pack_start(m_lang_label, true, false, 0);
        }
        public void set_lang_label(string str) {
            m_lang_label.set_text(str);
        }
    }

    private enum TravelDirection {
        NONE,
        BACKWARD,
    }

    private const uint EMOJI_GRID_PAGE = 10;

    // Set the actual default values in the constructor
    // because these fields are used for class_init() and static functions,
    // e.g. set_emoji_font(), can be called before class_init() is called.
    private static string m_current_lang_id;
    private static string m_emoji_font_family;
    private static int m_emoji_font_size;
    private static string[] m_favorites;
    private static int m_emoji_max_seq_len;
    private static GLib.HashTable<string, GLib.SList>?
            m_annotation_to_emojis_dict;
    private static GLib.HashTable<string, IBus.EmojiData>?
            m_emoji_to_data_dict;
    private static GLib.HashTable<string, GLib.SList>?
            m_category_to_emojis_dict;

    private ThemedRGBA m_rgba;
    private Gtk.Box m_vbox;
    private ETitleLabelBox m_title;
    private EEntry m_entry;
    private string? m_backward;
    private EScrolledWindow? m_scrolled_window = null;
    private EListBox m_list_box;
    private bool m_is_running = false;
    private string m_input_context_path = "";
    private GLib.MainLoop? m_loop;
    private string? m_result;
    private string? m_unicode_point = null;
    private bool m_candidate_panel_is_visible;
    int m_category_active_index;
    private IBus.LookupTable m_lookup_table;
    private Gtk.Label[] m_candidates;
    private bool m_enter_notify_enable = true;
    private uint m_entry_notify_show_id;
    private uint m_entry_notify_disable_id;

    public signal void candidate_clicked(uint index, uint button, uint state);

    public IBusEmojier() {
        GLib.Object(
            type : Gtk.WindowType.TOPLEVEL,
            events : Gdk.EventMask.KEY_PRESS_MASK |
                     Gdk.EventMask.KEY_RELEASE_MASK |
                     Gdk.EventMask.BUTTON_PRESS_MASK |
                     Gdk.EventMask.BUTTON_RELEASE_MASK,
            window_position : Gtk.WindowPosition.CENTER,
            icon_name: "ibus-setup",
            accept_focus : true,
            resizable : true,
            focus_visible : true
        );

        if (m_current_lang_id == null)
            m_current_lang_id = "en";
        if (m_emoji_font_family == null)
            m_emoji_font_family = "Monospace";
        if (m_emoji_font_size == 0)
            m_emoji_font_size = 16;
        if (m_favorites == null)
            m_favorites = {};

        Gdk.Display display = Gdk.Display.get_default();
        Gdk.Screen screen = (display != null) ?
                display.get_default_screen() : null;

        if (screen == null) {
            warning("Could not open display.");
            return;
        }
        m_rgba = new ThemedRGBA(this);
        uint bg_red = (uint)(m_rgba.normal_bg.red * 255);
        uint bg_green = (uint)(m_rgba.normal_bg.green * 255);
        uint bg_blue = (uint)(m_rgba.normal_bg.blue * 255);
        double bg_alpha = m_rgba.normal_bg.alpha;
        string data =
                "#IBusEmojierWhiteLabel { background-color: " +
                        "rgba(%u, %u, %u, %lf); ".printf(
                        bg_red, bg_green, bg_blue, bg_alpha) +
                "font-family: %s; font-size: %dpt; ".printf(
                        m_emoji_font_family, m_emoji_font_size) +
                "border-width: 4px; border-radius: 3px; } ";

        uint fg_red = (uint)(m_rgba.selected_fg.red * 255);
        uint fg_green = (uint)(m_rgba.selected_fg.green * 255);
        uint fg_blue = (uint)(m_rgba.selected_fg.blue * 255);
        double fg_alpha = m_rgba.selected_fg.alpha;
        bg_red = (uint)(m_rgba.selected_bg.red * 255);
        bg_green = (uint)(m_rgba.selected_bg.green * 255);
        bg_blue = (uint)(m_rgba.selected_bg.blue * 255);
        bg_alpha = m_rgba.selected_bg.alpha;
        data += "#IBusEmojierSelectedLabel { color: " +
                        "rgba(%u, %u, %u, %lf); ".printf(
                        fg_red, fg_green, fg_blue, fg_alpha) +
                "font-family: %s; font-size: %dpt; ".printf(
                        m_emoji_font_family, m_emoji_font_size) +
                "background-color: " +
                        "rgba(%u, %u, %u, %lf); ".printf(
                        bg_red, bg_green, bg_blue, bg_alpha) +
                "border-width: 4px; border-radius: 3px; }";

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

        m_title = new ETitleLabelBox(_("Emoji Choice"));
        set_titlebar(m_title);
        m_vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
        add(m_vbox);

        m_entry = new EEntry();
        m_entry.set_placeholder_text(_("Type annotation or choose emoji"));
        m_vbox.add(m_entry);
        m_entry.changed.connect(() => {
            update_candidate_window();
        });
        m_entry.icon_release.connect((icon_pos, event) => {
            hide_candidate_panel();
        });

        /* Set the accessible role of the label to a status bar so it
         * will emit name changed events that can be used by screen
         * readers.
         */
        Atk.Object obj = m_entry.get_accessible();
        obj.set_role (Atk.Role.STATUSBAR);

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

        if (m_annotation_to_emojis_dict == null) {
            reload_emoji_dict();
        }
    }


    private static void reload_emoji_dict() {
        init_emoji_dict();
        make_emoji_dict("en");
        if (m_current_lang_id != "en") {
            var lang_ids = m_current_lang_id.split("_");
            if (lang_ids.length > 1) {
                string sub_id = lang_ids[0];
                make_emoji_dict(sub_id);
            }
            make_emoji_dict(m_current_lang_id);
        }
    }


    private static void init_emoji_dict() {
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


    private static void make_emoji_dict(string lang) {
        GLib.SList<IBus.EmojiData> emoji_list = IBus.EmojiData.load(
                    Config.PKGDATADIR + "/dicts/emoji-" + lang + ".dict");
        if (emoji_list == null)
            return;
        foreach (IBus.EmojiData data in emoji_list) {
            update_emoji_to_data_dict(data, lang);
            update_annotation_to_emojis_dict(data);
            update_category_to_emojis_dict(data, lang);
        }
        GLib.List<unowned string> annotations =
                m_annotation_to_emojis_dict.get_keys();
        foreach (unowned string annotation in annotations) {
            if (m_emoji_max_seq_len < annotation.length)
                m_emoji_max_seq_len = annotation.length;
        }
    }


    private static void update_annotation_to_emojis_dict(IBus.EmojiData data) {
        string emoji = (data.get_emoji_alternates() != "") ?
                data.get_emoji_alternates() : data.get_emoji();
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


    private static string utf8_down(string str) {
        GLib.StringBuilder buff = new GLib.StringBuilder();
        int length = str.char_count();
        for (int i = 0; i < length; i++) {
            buff.append_unichar(str.get_char(0).tolower());
            str = str.next_char();
        }
        return buff.str;
    }


    private static string utf8_title(string str) {
        StringBuilder buff = new StringBuilder();
        int length = str.char_count();
        for (int i = 0; i < length; i++) {
            unichar ch = str.get_char(0);
            if (i == 0)
                buff.append_unichar(ch.toupper());
            else
                buff.append_unichar(ch);
            str = str.next_char();
        }
        return buff.str;
    }


    private static string utf8_code_point(string str) {
        StringBuilder buff = new StringBuilder();
        int length = str.char_count();
        for (int i = 0; i < length; i++) {
            unichar ch = str.get_char(0);
            if (i == 0)
                buff.append("U+%04X".printf(ch));
            else
                buff.append(" %04X".printf(ch));
            str = str.next_char();
        }
        return buff.str;
    }


    private static void update_emoji_to_data_dict(IBus.EmojiData data,
                                                  string         lang) {
        string emoji = (data.get_emoji_alternates() != "") ?
                data.get_emoji_alternates() : data.get_emoji();
        if (lang == "en") {
            string description = utf8_down(data.get_description());
            unowned GLib.SList<string> annotations = data.get_annotations();
            var words = description.split(" ");
            // If the description has less than 3 words, add it to annotations
            // FIXME: How to cast GLib.CompareFunc<string> to strcmp?
            if (words.length < 3 &&
                annotations.find_custom(
                        description,
                        GLib.strcmp) == null) {
                annotations.append(description);
                data.set_annotations(annotations.copy_deep(GLib.strdup));
            }
            m_emoji_to_data_dict.replace(emoji, data);
        } else {
            unowned IBus.EmojiData? en_data =
                    m_emoji_to_data_dict.lookup(emoji);
            if (en_data == null) {
                m_emoji_to_data_dict.insert(emoji, data);
                return;
            }
            string trans_description = data.get_description();
            en_data.set_description(trans_description);
            trans_description = utf8_down(trans_description);
            unowned GLib.SList<string> annotations = data.get_annotations();
            var words = trans_description.split(" ");
            // If the description has less than 3 words, add it to annotations
            // FIXME: How to cast GLib.CompareFunc<string> to strcmp?
            if (words.length < 3 &&
                annotations.find_custom(
                        trans_description,
                        GLib.strcmp) == null) {
                annotations.append(trans_description);
            }
            unowned GLib.SList<string> en_annotations
                = en_data.get_annotations();
            foreach (string annotation in en_annotations) {
                // FIXME: How to cast GLib.CompareFunc<string> to strcmp?
                if (annotations.find_custom(
                            annotation,
                            GLib.strcmp) == null) {
                    annotations.append(annotation.dup());
                }
            }
            en_data.set_annotations(annotations.copy_deep(GLib.strdup));
        }
    }


    private static void update_category_to_emojis_dict(IBus.EmojiData data,
                                                       string         lang) {
        string emoji = (data.get_emoji_alternates() != "") ?
                data.get_emoji_alternates() : data.get_emoji();
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
        resize(20, 1);
    }


    private void remove_all_children() {
        foreach (Gtk.Widget w in m_vbox.get_children()) {
            if (w.name == "IBusEmojierEntry" ||
                w.name == "IBusEmojierTitleLabelBox") {
                continue;
            }
            w.destroy();
        }
    }


    private void show_category_list() {
        remove_all_children();
        m_scrolled_window = new EScrolledWindow();
        set_fixed_size();

        string language =
            IBus.get_language_name(m_current_lang_id);
        m_title.set_lang_label(language);
        m_vbox.add(m_scrolled_window);
        Gtk.Viewport viewport = new Gtk.Viewport(null, null);
        m_scrolled_window.add(viewport);

        m_list_box = new EListBox();
        viewport.add(m_list_box);
        Gtk.Adjustment adjustment = m_scrolled_window.get_vadjustment();
        m_list_box.set_adjustment(adjustment);
        m_list_box.row_activated.connect((box, gtkrow) => {
            m_category_active_index = 0;
            EBoxRow row = gtkrow as EBoxRow;
            show_emoji_for_category(row);
        });

        uint n = 1;
        if (m_favorites.length > 0) {
            EBoxRow row = new EBoxRow("@favorites");
            EPaddedLabelBox widget =
                    new EPaddedLabelBox(_("Favorites"), Gtk.Align.CENTER);
            row.add(widget);
            m_list_box.add(row);
            if (n++ == m_category_active_index)
                m_list_box.select_row(row);
        }
        GLib.List<unowned string> categories =
                m_category_to_emojis_dict.get_keys();
        // FIXME: How to cast GLib.CompareFunc<string> to strcmp?
        categories.sort((a, b) => {
            return GLib.strcmp(_(a), _(b));
        });
        foreach (unowned string category in categories) {
            EBoxRow row = new EBoxRow(category);
            string locale_category = _(category);
            EPaddedLabelBox widget =
                    new EPaddedLabelBox(utf8_title(locale_category),
                                        Gtk.Align.CENTER);
            row.add(widget);
            m_list_box.add(row);
            if (n++ == m_category_active_index)
                m_list_box.select_row(row);
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
            m_backward = utf8_title(row.text);
        }
        show_candidate_panel();
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


    private bool check_unicode_point() {
        string annotation = m_entry.get_text();
        m_unicode_point = null;
        GLib.StringBuilder buff = new GLib.StringBuilder();
        GLib.StringBuilder retval = new GLib.StringBuilder();
        for (int i = 0; i < annotation.char_count(); i++) {
            unichar ch = annotation.get_char(i);
            if (ch == 0)
                return false;
            if (ch.isspace()) {
                unichar code = (unichar)buff.str.to_ulong(null, 16);
                buff.erase();
                if (!code.validate())
                    return false;
                retval.append(code.to_string());
                continue;
            }
            if (!ch.isxdigit())
                return false;
            buff.append_unichar(ch);
        }
        unichar code = (unichar)buff.str.to_ulong(null, 16);
        if (!code.validate())
            return false;
        retval.append(code.to_string());
        m_unicode_point = retval.str;
        if (m_unicode_point == null)
            return true;
        IBus.Text text = new IBus.Text.from_string(m_unicode_point);
        m_lookup_table.append_candidate(text);
        return true;
    }


    public void update_candidate_window() {
        string annotation = m_entry.get_text();
        if (annotation.length == 0) {
            hide_candidate_panel();
            m_backward = null;
            return;
        }
        annotation = utf8_down(annotation);
        if (annotation.length > m_emoji_max_seq_len) {
            hide_candidate_panel();
            return;
        }
        // Call check_unicode_point() to get m_unicode_point
        check_unicode_point();
        unowned GLib.SList<string>? emojis =
            m_annotation_to_emojis_dict.lookup(annotation);
        if (emojis == null && m_unicode_point == null) {
            hide_candidate_panel();
            return;
        }
        m_lookup_table.clear();
        // Call check_unicode_point() to update m_lookup_table
        check_unicode_point();
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
                    "%s (%u / %u)".printf(m_backward, cursor, ncandidates - 1);
            EPaddedLabelBox label =
                    new EPaddedLabelBox(backward_desc,
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
            Gtk.Label label;
            if (i == cursor)
                label = new ESelectedLabel(candidate.text) as Gtk.Label;
            else
                label = new EWhiteLabel(candidate.text) as Gtk.Label;
            if (candidate.text.char_count() > 2) {
                string font_family = m_emoji_font_family;
                int font_size = m_emoji_font_size - 2;
                string emoji_font = "%s %d".printf(font_family, font_size);
                string markup = "<span font=\"%s\">%s</span>".
                        printf(emoji_font, candidate.get_text());
                label.set_markup(markup);
            }
            label.set_halign(Gtk.Align.FILL);
            label.set_valign(Gtk.Align.FILL);
            Gtk.EventBox candidate_ebox = new Gtk.EventBox();
            candidate_ebox.add(label);
            // Make a copy of i to workaround a bug in vala.
            // https://bugzilla.gnome.org/show_bug.cgi?id=628336
            uint index = i;
            candidate_ebox.button_press_event.connect((w, e) => {
                candidate_clicked(index, e.button, e.state);
                return true;
            });
            candidate_ebox.enter_notify_event.connect((e) => {
                // m_enter_notify_enable is added because
                // enter_notify_event conflicts with keyboard operations.
                if (!m_enter_notify_enable)
                    return true;
                if (m_lookup_table.get_cursor_pos() == index)
                    return true;
                m_lookup_table.set_cursor_pos(index);
                if (m_entry_notify_show_id > 0) {
                        GLib.Source.remove(m_entry_notify_show_id);
                }
                // If timeout is not added, memory leak happens and
                // button_press_event signal does not work above.
                m_entry_notify_show_id = GLib.Timeout.add(100, () => {
                        show_candidate_panel();
                        return false;
                });
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
            unowned IBus.EmojiData? data =
                    m_emoji_to_data_dict.lookup(candidate.text);
            if (data == null) {
                // TODO: Provide a custom description and annotation for
                // the favorite emojis.
                EPaddedLabelBox widget = new EPaddedLabelBox(
                        _("Description: %s").printf(_("None")),
                        Gtk.Align.START);
                m_vbox.add(widget);
                widget.show_all();
                EPaddedLabelBox widget_code = new EPaddedLabelBox(
                        _("Code point: %s").printf(
                                utf8_code_point(candidate.text)),
                        Gtk.Align.START);
                m_vbox.add(widget_code);
                widget_code.show_all();
                return;
            } else {
                unowned string description = data.get_description();
                EPaddedLabelBox widget = new EPaddedLabelBox(
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
                    EPaddedLabelBox widget =
                            new EPaddedLabelBox(buff.str,
                                                Gtk.Align.START);
                    m_vbox.add(widget);
                    widget.show_all();
                    buff.erase();
                }
            }
            if (buff.str != "") {
                EPaddedLabelBox widget = new EPaddedLabelBox(buff.str,
                                                             Gtk.Align.START);
                m_vbox.add(widget);
                widget.show_all();
            }
            EPaddedLabelBox widget_code = new EPaddedLabelBox(
                    _("Code point: %s").printf(
                            utf8_code_point(candidate.text)),
                    Gtk.Align.START);
            m_vbox.add(widget_code);
            widget_code.show_all();
        }
    }


    private void hide_candidate_panel() {
        m_enter_notify_enable = true;
        m_candidate_panel_is_visible = false;
        if (m_loop.is_running())
            show_category_list();
    }


    private void enter_notify_disable_with_timer() {
        // Enable keyboard operation and disable mouse operation.
        m_enter_notify_enable = false;
        if (m_entry_notify_disable_id > 0) {
            GLib.Source.remove(m_entry_notify_disable_id);
        }
        // Bring back the mouse operation after a timeout.
        m_entry_notify_show_id = GLib.Timeout.add(100, () => {
            m_enter_notify_enable = true;
            return false;
        });
    }


    private void candidate_panel_cursor_down() {
        enter_notify_disable_with_timer();
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
        enter_notify_disable_with_timer();
        int ncandidates = (int)m_lookup_table.get_number_of_candidates();
        int cursor = (int)m_lookup_table.get_cursor_pos();
        int highest_pos =
            ((ncandidates - 1)/ (int)EMOJI_GRID_PAGE * (int)EMOJI_GRID_PAGE)
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


    private bool key_press_cursor_horizontal(uint keyval,
                                             uint modifiers) {
        assert (keyval == Gdk.Key.Left || keyval == Gdk.Key.Right);

        uint ncandidates = m_lookup_table.get_number_of_candidates();
        if (m_candidate_panel_is_visible && ncandidates > 1) {
            enter_notify_disable_with_timer();
            if (keyval == Gdk.Key.Left)
                m_lookup_table.cursor_up();
            else if (keyval == Gdk.Key.Right)
                m_lookup_table.cursor_down();
            show_candidate_panel();
        } else if (m_entry.get_text().len() > 0) {
            int step = 0;
            if (keyval == Gdk.Key.Left)
                step = -1;
            else if (keyval == Gdk.Key.Right)
                step = 1;
            GLib.Signal.emit_by_name(
                    m_entry, "move-cursor",
                    Gtk.MovementStep.VISUAL_POSITIONS,
                    step,
                    (modifiers & Gdk.ModifierType.SHIFT_MASK) != 0
                            ? true : false);
        } else {
            // For Gdk.Key.f and Gdk.Key.b
            if (keyval == Gdk.Key.Left)
                keyval = Gdk.Key.Up;
            else if (keyval == Gdk.Key.Right)
                keyval = Gdk.Key.Down;
            category_list_cursor_move(keyval);
        }
        return true;
    }


    private bool key_press_cursor_vertical(uint keyval) {
        assert (keyval == Gdk.Key.Down || keyval == Gdk.Key.Up);

        uint ncandidates = m_lookup_table.get_number_of_candidates();
        if (m_candidate_panel_is_visible && ncandidates > 1) {
            if (keyval == Gdk.Key.Down)
                candidate_panel_cursor_down();
            else if (keyval == Gdk.Key.Up)
                candidate_panel_cursor_up();
        } else {
            category_list_cursor_move(keyval);
        }
        return true;
    }


    private bool key_press_cursor_home_end(uint keyval,
                                           uint modifiers) {
        assert (keyval == Gdk.Key.Home || keyval == Gdk.Key.End);

        uint ncandidates = m_lookup_table.get_number_of_candidates();
        if (m_candidate_panel_is_visible && ncandidates > 1) {
            enter_notify_disable_with_timer();
            if (keyval == Gdk.Key.Home) {
                m_lookup_table.set_cursor_pos(0);
            } else if (keyval == Gdk.Key.End) {
                m_lookup_table.set_cursor_pos(ncandidates - 1);
            }
            show_candidate_panel();
            return true;
        }
        if (m_entry.get_text().len() > 0) {
            int step = 0;
            if (keyval == Gdk.Key.Home)
                step = -1;
            else if (keyval == Gdk.Key.End)
                step = 1;
            GLib.Signal.emit_by_name(
                    m_entry, "move-cursor",
                    Gtk.MovementStep.DISPLAY_LINE_ENDS,
                    step,
                    (modifiers & Gdk.ModifierType.SHIFT_MASK) != 0
                            ? true : false);
            return true;
        }
        return false;
    }


    private bool key_press_cursor_escape() {
        if (m_candidate_panel_is_visible) {
            hide_candidate_panel();
            return true;
        } else if (m_entry.get_text().length == 0) {
            m_loop.quit();
            hide_candidate_panel();
            return true;
        }
        m_entry.delete_text(0, -1);
        return true;
    }


    private void entry_enter_keyval(uint keyval) {
        unichar ch = IBus.keyval_to_unicode(keyval);
        if (ch.iscntrl())
            return;
        string str = ch.to_string();

        // what gtk_entry_commit_cb() do
        if (m_entry.get_selection_bounds(null, null)) {
            m_entry.delete_selection();
        } else {
            if (m_entry.get_overwrite_mode()) {
               uint text_length = m_entry.get_buffer().get_length();
               if (m_entry.cursor_position < text_length)
                   m_entry.delete_from_cursor(Gtk.DeleteType.CHARS, 1);
            }
        }
        int pos = m_entry.get_position();
        m_entry.insert_text(str, -1, ref pos);
        m_entry.set_position(pos);
    }


    public string run(string input_context_path) {
        assert (m_loop == null);

        m_is_running = true;
        m_input_context_path = input_context_path;
        m_candidate_panel_is_visible = false;
        m_result = null;
        m_enter_notify_enable = true;

        /* Let gtk recalculate the window size. */
        resize(1, 1);

        m_entry.set_text("");
        //m_entry.grab_focus();

        show_category_list();
        m_entry.set_activates_default(true);
        show_all();

        m_loop = new GLib.MainLoop();
        m_loop.run();
        m_loop = null;

        // Need focus-out on Gtk.Entry to send the emoji to applications.
        Gdk.Event fevent = new Gdk.Event(Gdk.EventType.FOCUS_CHANGE);
        fevent.focus_change.in = 0;
        fevent.focus_change.window  = get_window();
        m_entry.send_focus_change(fevent);
        fevent.focus_change.window  = null;

        hide();
        // Make sure the switcher is hidden before returning from this function.
        while (Gtk.events_pending())
            Gtk.main_iteration();
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
        switch (keyval) {
        case Gdk.Key.Escape:
            if (key_press_cursor_escape())
                return true;
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
            if (m_entry.get_text().len() > 0) {
                GLib.Signal.emit_by_name(m_entry, "backspace");
            }
            break;
        case Gdk.Key.space:
        case Gdk.Key.KP_Space:
            if ((modifiers & Gdk.ModifierType.SHIFT_MASK) != 0) {
                if (m_entry.get_text().len() > 0)
                    entry_enter_keyval(keyval);
            } else if (m_candidate_panel_is_visible) {
                enter_notify_disable_with_timer();
                m_lookup_table.cursor_down();
                show_candidate_panel();
            }
            else {
                category_list_cursor_move(Gdk.Key.Down);
            }
            return true;
        case Gdk.Key.Right:
            key_press_cursor_horizontal(keyval, modifiers);
            return true;
        case Gdk.Key.Left:
            key_press_cursor_horizontal(keyval, modifiers);
            return true;
        case Gdk.Key.Down:
            key_press_cursor_vertical(keyval);
            return true;
        case Gdk.Key.Up:
            key_press_cursor_vertical(keyval);
            return true;
        case Gdk.Key.Page_Down:
            if (m_candidate_panel_is_visible) {
                enter_notify_disable_with_timer();
                m_lookup_table.page_down();
                show_candidate_panel();
                return true;
            }
            break;
        case Gdk.Key.Page_Up:
            if (m_candidate_panel_is_visible) {
                enter_notify_disable_with_timer();
                m_lookup_table.page_up();
                show_candidate_panel();
                return true;
            }
            break;
        case Gdk.Key.Home:
            if (key_press_cursor_home_end(keyval, modifiers))
                return true;
            break;
        case Gdk.Key.End:
            if (key_press_cursor_home_end(keyval, modifiers))
                return true;
            break;
        }

        if ((modifiers & Gdk.ModifierType.CONTROL_MASK) != 0) {
            switch (keyval) {
            case Gdk.Key.f:
                key_press_cursor_horizontal(Gdk.Key.Right, modifiers);
                return true;
            case Gdk.Key.b:
                key_press_cursor_horizontal(Gdk.Key.Left, modifiers);
                return true;
            case Gdk.Key.n:
                key_press_cursor_vertical(Gdk.Key.Down);
                return true;
            case Gdk.Key.p:
                key_press_cursor_vertical(Gdk.Key.Up);
                return true;
            case Gdk.Key.h:
                if (key_press_cursor_home_end(Gdk.Key.Home, modifiers))
                    return true;
                break;
            case Gdk.Key.e:
                if (key_press_cursor_home_end(Gdk.Key.End, modifiers))
                    return true;
                break;
            case Gdk.Key.u:
                if (key_press_cursor_escape())
                    return true;
                break;
            case Gdk.Key.a:
                if (m_entry.get_text().len() > 0) {
                    m_entry.select_region(0, -1);
                    return true;
                }
                break;
            }
            return false;
        }

        entry_enter_keyval(keyval);
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


    public void present_centralize() {
        present();
        m_entry.set_activates_default(true);
        Gtk.Allocation allocation;
        get_allocation(out allocation);
        Gdk.Screen screen = Gdk.Screen.get_default();
        int monitor_num = screen.get_monitor_at_window(get_window());
        Gdk.Rectangle monitor_area;
        screen.get_monitor_geometry(monitor_num, out monitor_area);
        int x = (monitor_area.x + monitor_area.width - allocation.width)/2;
        int y = (monitor_area.y + monitor_area.height
                 - allocation.height)/2;
        move(x, y);
    }


    public static bool has_loaded_emoji_dict() {
        if (m_emoji_to_data_dict == null)
            return false;
        GLib.List keys = m_emoji_to_data_dict.get_keys();
        if (keys.length() == 0)
            return false;
        return true;
    }


    public static void set_annotation_lang(string? lang) {
        if (lang == null || lang == "")
            lang = "en";
        if (m_current_lang_id == lang)
            return;
        m_current_lang_id = lang;
        reload_emoji_dict();
    }


    public static void set_emoji_font(string? emoji_font) {
        return_if_fail(emoji_font != null && emoji_font != "");
        Pango.FontDescription font_desc =
                Pango.FontDescription.from_string(emoji_font);
        string font_family = font_desc.get_family();
        if (font_family != null)
            m_emoji_font_family = font_family;
        int font_size = font_desc.get_size() / Pango.SCALE;
        if (font_size != 0)
            m_emoji_font_size = font_size;
    }


    public static void set_favorites(string[]? unowned_favorites) {
        m_favorites = {};
        foreach (string favorite in unowned_favorites) {
            m_favorites += favorite;
        }
    }
}
