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

class IBusEmojier : Gtk.ApplicationWindow {
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
            this.motion_notify_event.connect((e) => {
#if VALA_0_24
                Gdk.EventMotion pe = e;
#else
                Gdk.EventMotion *pe = &e;
#endif
                if (m_mouse_x == pe.x_root && m_mouse_y == pe.y_root)
                    return false;
                m_mouse_x = pe.x_root;
                m_mouse_y = pe.y_root;
                var row = this.get_row_at_y((int)e.y);
                if (row != null)
                    this.select_row(row);
                return false;
            });
            this.enter_notify_event.connect((e) => {
                // avoid gtk_button_update_state()
                return true;
            });
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
    private class EGoldLabel : Gtk.Label {
        public EGoldLabel(string text) {
            GLib.Object(
                name : "IBusEmojierGoldLabel"
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
            label.get_style_context().add_class(Gtk.STYLE_CLASS_TITLE);
            vbox.pack_start(label, true, false, 0);
            m_lang_label = new Gtk.Label(null);
            m_lang_label.get_style_context().add_class(
                    Gtk.STYLE_CLASS_SUBTITLE);
            vbox.pack_start(m_lang_label, true, false, 0);

            var menu = new GLib.Menu();
            menu.append(_("Show emoji variants"), "win.variant");
            var menu_button = new Gtk.MenuButton();
            menu_button.set_direction(Gtk.ArrowType.NONE);
            menu_button.set_valign(Gtk.Align.CENTER);
            menu_button.set_menu_model(menu);
            menu_button.set_tooltip_text(_("Menu"));
            pack_end(menu_button);
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
    private const string EMOJI_CATEGORY_FAVORITES = N_("Favorites");
    private const string EMOJI_CATEGORY_OTHERS = N_("Others");
    private const unichar[] EMOJI_VARIANT_LIST = {
            0x1f3fb, 0x1f3fc, 0x1f3fd, 0x1f3fe, 0x1f3ff, 0x200d };

    // Set the actual default values in the constructor
    // because these fields are used for class_init() and static functions,
    // e.g. set_emoji_font(), can be called before class_init() is called.
    private static string m_current_lang_id;
    private static string m_emoji_font_family;
    private static int m_emoji_font_size;
    private static string[] m_favorites;
    private static string[] m_favorite_annotations;
    private static int m_emoji_max_seq_len;
    private static bool m_has_partial_match;
    private static uint m_partial_match_length;
    private static uint m_partial_match_condition;
    private static bool m_show_emoji_variant = false;
    private static GLib.HashTable<string, GLib.SList<string>>?
            m_annotation_to_emojis_dict;
    private static GLib.HashTable<string, IBus.EmojiData>?
            m_emoji_to_data_dict;
    private static GLib.HashTable<string, GLib.SList<string>>?
            m_category_to_emojis_dict;
    private static GLib.HashTable<string, GLib.SList<string>>?
            m_emoji_to_emoji_variants_dict;

    private ThemedRGBA m_rgba;
    private Gtk.Box m_vbox;
    private ETitleLabelBox m_title;
    private EEntry m_entry;
    private string? m_backward;
    private int m_backward_index = -1;
    private EScrolledWindow? m_scrolled_window = null;
    private EListBox m_list_box;
    private bool m_is_running = false;
    private string m_input_context_path = "";
    private GLib.MainLoop? m_loop;
    private string? m_result;
    private string? m_unicode_point = null;
    private bool m_candidate_panel_is_visible;
    private int m_category_active_index;
    private IBus.LookupTable m_lookup_table;
    private Gtk.Label[] m_candidates;
    private bool m_enter_notify_enable = true;
    private uint m_entry_notify_show_id;
    private uint m_entry_notify_disable_id;
    protected static double m_mouse_x;
    protected static double m_mouse_y;

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

        // GLib.ActionEntry accepts const variables only.
        var action = new GLib.SimpleAction.stateful(
                "variant",
                null,
                new GLib.Variant.boolean(m_show_emoji_variant));
        action.activate.connect(check_action_variant_cb);
        add_action(action);
        if (m_current_lang_id == null)
            m_current_lang_id = "en";
        if (m_emoji_font_family == null)
            m_emoji_font_family = "Monospace";
        if (m_emoji_font_size == 0)
            m_emoji_font_size = 16;
        if (m_favorites == null)
            m_favorites = {};
        if (m_favorite_annotations == null)
            m_favorite_annotations = {};

        Gdk.Display display = Gdk.Display.get_default();
        Gdk.Screen screen = (display != null) ?
                display.get_default_screen() : null;

        if (screen == null) {
            warning("Could not open display.");
            return;
        }
        // Set en locale because de_DE's decimal_point is ',' instead of '.'
        string? backup_locale =
            Intl.setlocale(LocaleCategory.NUMERIC, null).dup();
        if (Intl.setlocale(LocaleCategory.NUMERIC, "en_US.UTF-8") == null) {
          if (Intl.setlocale(LocaleCategory.NUMERIC, "C.UTF-8") == null) {
              if (Intl.setlocale(LocaleCategory.NUMERIC, "C") == null) {
                  warning("You don't install either en_US.UTF-8 or C.UTF-8 " +
                          "or C locale");
              }
          }
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
        data += "#IBusEmojierGoldLabel { color: " +
                        "rgba(%u, %u, %u, %lf); ".printf(
                        fg_red, fg_green, fg_blue, fg_alpha) +
                "font-family: %s; font-size: %dpt; ".printf(
                        m_emoji_font_family, m_emoji_font_size) +
                "background-color: #b09c5f; " +
                "border-width: 4px; border-radius: 3px; }";

        Gtk.CssProvider css_provider = new Gtk.CssProvider();
        try {
            css_provider.load_from_data(data, -1);
        } catch (GLib.Error e) {
            warning("Failed css_provider_from_data: %s", e.message);
            return;
        }
        if (backup_locale != null)
            Intl.setlocale(LocaleCategory.NUMERIC, backup_locale);
        else
            Intl.setlocale(LocaleCategory.NUMERIC, "");

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
            candidate_panel_select_index(i);
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
        update_favorite_emoji_dict();
    }


    private static void init_emoji_dict() {
        m_annotation_to_emojis_dict =
                new GLib.HashTable<string, GLib.SList<string>>(GLib.str_hash,
                                                               GLib.str_equal);
        m_emoji_to_data_dict =
                new GLib.HashTable<string, IBus.EmojiData>(GLib.str_hash,
                                                           GLib.str_equal);
        m_category_to_emojis_dict =
                new GLib.HashTable<string, GLib.SList<string>>(GLib.str_hash,
                                                               GLib.str_equal);
        m_emoji_to_emoji_variants_dict =
                new GLib.HashTable<string, GLib.SList<string>>(GLib.str_hash,
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
        string emoji = data.get_emoji();
        unowned GLib.SList<string> annotations = data.get_annotations();
        foreach (string annotation in annotations) {
            bool has_emoji = false;
            GLib.SList<string> hits =
                    m_annotation_to_emojis_dict.lookup(annotation).copy_deep(
                            GLib.strdup);
            foreach (string hit_emoji in hits) {
                if (hit_emoji == emoji) {
                    has_emoji = true;
                    break;
                }
            }
            if (!has_emoji) {
                hits.append(emoji);
                m_annotation_to_emojis_dict.replace(
                        annotation,
                        hits.copy_deep(GLib.strdup));
            }
        }
    }


    private static string utf8_code_point(string str) {
        var buff = new GLib.StringBuilder();
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


    private static string utf8_entity(string str) {
        var buff = new GLib.StringBuilder();
        int length = str.char_count();
        for (int i = 0; i < length; i++) {
            unichar ch = str.get_char(0);
            switch(ch) {
            case '<':
                buff.append("&lt;");
                break;
            case '>':
                buff.append("&gt;");
                break;
            case '&':
                buff.append("&amp;");
                break;
            default:
                buff.append_unichar(ch);
                break;
            }
            str = str.next_char();
        }
        return buff.str;
    }


    private static void
    update_annotations_with_description (IBus.EmojiData data,
                                         string         description) {
        GLib.SList<string> annotations =
                data.get_annotations().copy_deep(GLib.strdup);
        bool update_annotations = false;
        string former = null;
        string later = null;
        int index = description.index_of(": ");
        if (index > 0) {
            former = description.substring(0, index);
            if (annotations.find_custom(former, GLib.strcmp) == null) {
                annotations.append(former);
                update_annotations = true;
            }
            later = description.substring(index + 2);
        } else {
            later = description.dup();
        }
        var words = later.split(" ");
        // If the description has less than 3 words, add it to annotations
        // FIXME: How to cast GLib.CompareFunc<string> to strcmp?
        if (words.length < 3 &&
            annotations.find_custom(
                    later,
                    GLib.strcmp) == null) {
            annotations.append(later);
            update_annotations = true;
        }
        if (update_annotations)
            data.set_annotations(annotations.copy_deep(GLib.strdup));
    }


    private static void update_emoji_to_data_dict(IBus.EmojiData data,
                                                  string         lang) {
        string emoji = data.get_emoji();
        if (lang == "en") {
            string description = data.get_description().down();
            update_annotations_with_description (data, description);
            m_emoji_to_data_dict.replace(emoji, data);
        } else {
            unowned IBus.EmojiData? en_data = null;
            en_data = m_emoji_to_data_dict.lookup(emoji);
            if (en_data == null) {
                m_emoji_to_data_dict.insert(emoji, data);
                return;
            }
            string trans_description = data.get_description();
            en_data.set_description(trans_description);
            trans_description = trans_description.down();
            update_annotations_with_description (data, trans_description);
            unowned GLib.SList<string> annotations = data.get_annotations();
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
        string emoji = data.get_emoji();
        string category = data.get_category();
        if (category == "")
            category = EMOJI_CATEGORY_OTHERS;
        if (lang == "en") {
            bool has_variant = false;
            foreach (unichar ch in EMOJI_VARIANT_LIST) {
                if (emoji.index_of_char(ch) >= 0) {
                    has_variant = true;
                    break;
                }
            }
            // If emoji includes variants (skin colors and items),
            // it's escaped in m_emoji_to_emoji_variants_dict and
            // not shown by default.
            if (has_variant) {
                unichar base_ch = emoji.get_char();
                string base_emoji  = base_ch.to_string();
                var buff = new GLib.StringBuilder();
                buff.append_unichar(base_ch);
                buff.append_unichar(0xfe0f);
                if (m_emoji_to_data_dict.lookup(buff.str) != null)
                    base_emoji = buff.str;
                GLib.SList<string>? variants =
                        m_emoji_to_emoji_variants_dict.lookup(
                                base_emoji).copy_deep(GLib.strdup);
                if (variants.find_custom(emoji, GLib.strcmp) == null) {
                    if (variants == null)
                        variants.append(base_emoji);
                    variants.append(emoji);
                    m_emoji_to_emoji_variants_dict.replace(
                            base_emoji,
                            variants.copy_deep(GLib.strdup));
                }
                return;
            }
            bool has_emoji = false;
            GLib.SList<string> hits =
                    m_category_to_emojis_dict.lookup(category).copy_deep(
                            GLib.strdup);
            foreach (string hit_emoji in hits) {
                if (hit_emoji == emoji) {
                    has_emoji = true;
                    break;
                }
            }
            if (!has_emoji) {
                hits.append(emoji);
                m_category_to_emojis_dict.replace(category,
                                                  hits.copy_deep(GLib.strdup));
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
            show_emoji_for_category(row.text);
        });

        uint n = 1;
        if (m_favorites.length > 0) {
            EBoxRow row = new EBoxRow(EMOJI_CATEGORY_FAVORITES);
            EPaddedLabelBox widget =
                    new EPaddedLabelBox(_(EMOJI_CATEGORY_FAVORITES),
                                        Gtk.Align.CENTER);
            row.add(widget);
            m_list_box.add(row);
            if (n++ == m_category_active_index)
                m_list_box.select_row(row);
        }
        GLib.List<unowned string> categories =
                m_category_to_emojis_dict.get_keys();
        // FIXME: How to cast GLib.CompareFunc<string> to strcmp?
        categories.sort((a, b) => {
            if (a == EMOJI_CATEGORY_OTHERS && b != EMOJI_CATEGORY_OTHERS)
                return 1;
            else if (a != EMOJI_CATEGORY_OTHERS && b == EMOJI_CATEGORY_OTHERS)
                return -1;
            return GLib.strcmp(_(a), _(b));
        });
        foreach (unowned string category in categories) {
            // "Others" category includes next unicode chars and fonts do not support
            // the base and varints yet.
            if (category == EMOJI_CATEGORY_OTHERS)
               continue;
            EBoxRow row = new EBoxRow(category);
            EPaddedLabelBox widget =
                    new EPaddedLabelBox(_(category), Gtk.Align.CENTER);
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


    private void show_emoji_for_category(string category) {
        if (category == EMOJI_CATEGORY_FAVORITES) {
            m_lookup_table.clear();
            foreach (unowned string favorate in m_favorites) {
                IBus.Text text = new IBus.Text.from_string(favorate);
                m_lookup_table.append_candidate(text);
            }
            m_backward = category;
        } else {
            unowned GLib.SList<unowned string> emojis =
                    m_category_to_emojis_dict.lookup(category);
            m_lookup_table.clear();
            foreach (unowned string emoji in emojis) {
                IBus.Text text = new IBus.Text.from_string(emoji);
                m_lookup_table.append_candidate(text);
            }
            m_backward = category;
        }
        // Restore the cursor position before the special table of
        // emoji variants is shown.
        if (m_backward_index >= 0) {
            m_lookup_table.set_cursor_pos((uint)m_backward_index);
            m_backward_index = -1;
        }
        show_candidate_panel();
    }


    private void show_emoji_variants(GLib.SList<string>? emojis) {
        m_backward_index = (int)m_lookup_table.get_cursor_pos();
        m_lookup_table.clear();
        foreach (unowned string emoji in emojis) {
            IBus.Text text = new IBus.Text.from_string(emoji);
            m_lookup_table.append_candidate(text);
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
        // Add "0x" because uint64.ascii_strtoull() is not accessible
        // and need to use uint64.parse()
        var buff = new GLib.StringBuilder("0x");
        var retval = new GLib.StringBuilder();
        for (int i = 0; i < annotation.char_count(); i++) {
            unichar ch = annotation.get_char(i);
            if (ch == 0)
                return false;
            if (ch.isspace()) {
                unichar code = (unichar)uint64.parse(buff.str);
                buff.assign("0x");
                if (!code.validate())
                    return false;
                retval.append(code.to_string());
                continue;
            }
            if (!ch.isxdigit())
                return false;
            buff.append_unichar(ch);
        }
        unichar code = (unichar)uint64.parse(buff.str);
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


    private GLib.SList<string>?
    lookup_emojis_from_annotation(string annotation) {
        GLib.SList<string>? total_emojis = null;
        unowned GLib.SList<string>? sub_emojis = null;
        int length = annotation.length;
        if (m_has_partial_match && length >= m_partial_match_length) {
            foreach (unowned string key in
                     m_annotation_to_emojis_dict.get_keys()) {
                if (key.length < length)
                    continue;
                bool matched = false;
                switch(m_partial_match_condition) {
                case 0:
                    if (key.has_prefix(annotation))
                        matched = true;
                    break;
                case 1:
                    if (key.has_suffix(annotation))
                        matched = true;
                    break;
                case 2:
                    if (key.index_of(annotation) >= 0)
                        matched = true;
                    break;
                default:
                    break;
                }
                if (!matched)
                    continue;
                sub_emojis = m_annotation_to_emojis_dict.lookup(key);
                foreach (unowned string emoji in sub_emojis) {
                    if (total_emojis.find_custom(emoji, GLib.strcmp) == null) {
                        total_emojis.append(emoji);
                    }
                }
            }
        } else {
            sub_emojis = m_annotation_to_emojis_dict.lookup(annotation);
            foreach (unowned string emoji in sub_emojis)
                total_emojis.append(emoji);
        }
        return total_emojis;
    }

    private void update_candidate_window() {
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
        check_unicode_point();
        GLib.SList<string>? total_emojis =
            lookup_emojis_from_annotation(annotation);
        if (total_emojis == null) {
            /* Users can type title strings against lower case.
             * E.g. "Smile" against "smile"
             * But the dictionary has the case sensitive annotations.
             * E.g. ":D" and ":q"
             * So need to call lookup_emojis_from_annotation() twice.
             */
            annotation = annotation.down();
            total_emojis = lookup_emojis_from_annotation(annotation);
        }
        if (total_emojis == null && m_unicode_point == null) {
            hide_candidate_panel();
            return;
        }
        m_lookup_table.clear();
        // Call check_unicode_point() to update m_lookup_table
        check_unicode_point();
        foreach (unowned string emoji in total_emojis) {
            IBus.Text text = new IBus.Text.from_string(emoji);
            m_lookup_table.append_candidate(text);
        }
        show_candidate_panel();
    }


    private void show_code_point_description(string text) {
        EPaddedLabelBox widget_code = new EPaddedLabelBox(
                    _("Code point: %s").printf(utf8_code_point(text)),
                    Gtk.Align.START);
        m_vbox.add(widget_code);
        widget_code.show_all();
        if (m_emoji_to_emoji_variants_dict.lookup(text) != null) {
            EPaddedLabelBox widget_has_variant = new EPaddedLabelBox(
                    _("Has emoji variants"),
                    Gtk.Align.START);
            m_vbox.add(widget_has_variant);
            widget_has_variant.show_all();
        }
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
                    "%s (%u / %u)".printf(_(m_backward), cursor, ncandidates - 1);
            EPaddedLabelBox label =
                    new EPaddedLabelBox(backward_desc,
                                        Gtk.Align.CENTER,
                                        TravelDirection.BACKWARD);
            Gtk.Button button = new Gtk.Button();
            button.add(label);
            m_vbox.add(button);
            button.show_all();
            button.button_press_event.connect((w, e) => {
                // Bring back to emoji candidate panel in case
                // m_show_emoji_variant is enabled and shows variants.
                if (m_backward_index >= 0 && m_backward != null)
                    show_emoji_for_category(m_backward);
                else
                    hide_candidate_panel();
                return true;
            });
        }
        EGrid grid = new EGrid();
        int n = 0;
        for (uint i = page_start_pos; i < page_end_pos; i++) {
            string text = m_lookup_table.get_candidate(i).text;
            bool has_variant =
                    (m_emoji_to_emoji_variants_dict.lookup(text) != null);
            Gtk.Label label;
            // If 'i' is the cursor position, use the selected color.
            // If the emoji has emoji variants, use the gold color.
            // Otherwise the white color.
            if (i == cursor) {
                label = new ESelectedLabel(text) as Gtk.Label;
            } else if (m_show_emoji_variant && has_variant &&
                       m_backward_index < 0) {
                label = new EGoldLabel(text) as Gtk.Label;
            } else {
                label = new EWhiteLabel(text) as Gtk.Label;
            }
            if (text.char_count() > 2) {
                string font_family = m_emoji_font_family;
                int font_size = m_emoji_font_size - 2;
                string emoji_font = "%s %d".printf(font_family, font_size);
                string markup = "<span font=\"%s\">%s</span>".
                        printf(emoji_font, utf8_entity(text));
                label.set_markup(markup);
            }
            label.set_halign(Gtk.Align.FILL);
            label.set_valign(Gtk.Align.FILL);
            Gtk.EventBox candidate_ebox = new Gtk.EventBox();
            candidate_ebox.add_events(Gdk.EventMask.POINTER_MOTION_MASK);
            candidate_ebox.add(label);
            // Make a copy of i to workaround a bug in vala.
            // https://bugzilla.gnome.org/show_bug.cgi?id=628336
            uint index = i;
            candidate_ebox.button_press_event.connect((w, e) => {
                candidate_clicked(index, e.button, e.state);
                return true;
            });
            candidate_ebox.motion_notify_event.connect((e) => {
                // m_enter_notify_enable is added because
                // enter_notify_event conflicts with keyboard operations.
                if (!m_enter_notify_enable)
                    return false;
                if (m_lookup_table.get_cursor_pos() == index)
                    return false;
#if VALA_0_24
                Gdk.EventMotion pe = e;
#else
                Gdk.EventMotion *pe = &e;
#endif
                if (m_mouse_x == pe.x_root && m_mouse_y == pe.y_root)
                    return false;
                m_mouse_x = pe.x_root;
                m_mouse_y = pe.y_root;

                m_lookup_table.set_cursor_pos(index);
                if (m_entry_notify_show_id > 0 &&
                    GLib.MainContext.default().find_source_by_id(
                            m_entry_notify_show_id) != null) {
                        GLib.Source.remove(m_entry_notify_show_id);
                }
                // If timeout is not added, memory leak happens and
                // button_press_event signal does not work above.
                m_entry_notify_show_id = GLib.Timeout.add(100, () => {
                        show_candidate_panel();
                        return false;
                });
                return false;
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
            string text = m_lookup_table.get_candidate(cursor).text;
            unowned IBus.EmojiData? data = m_emoji_to_data_dict.lookup(text);
            if (data == null) {
                // TODO: Provide a custom description and annotation for
                // the favorite emojis.
                EPaddedLabelBox widget = new EPaddedLabelBox(
                        _("Description: %s").printf(_("None")),
                        Gtk.Align.START);
                m_vbox.add(widget);
                widget.show_all();
                show_code_point_description(text);
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
            var buff = new GLib.StringBuilder();
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
            show_code_point_description(text);
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


    private void candidate_panel_select_index(uint index) {
        string text = m_lookup_table.get_candidate(index).text;
        unowned GLib.SList<string>? emojis =
                m_emoji_to_emoji_variants_dict.lookup(text);
        if (m_show_emoji_variant && emojis != null &&
            m_backward_index < 0) {
            show_emoji_variants(emojis);
        } else {
            m_result = text;
            m_loop.quit();
            hide_candidate_panel();
        }
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
        m_scrolled_window.set_hadjustment(new Gtk.Adjustment(0, 0, 0, 0, 0, 0));
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
        } else if (m_entry.get_text().length > 0) {
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
        if (m_entry.get_text().length > 0) {
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


    private bool key_press_escape() {
        if (m_backward_index >= 0 && m_backward != null) {
            show_emoji_for_category(m_backward);
            return true;
        } else if (m_candidate_panel_is_visible) {
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


    private bool key_press_enter() {
        if (m_candidate_panel_is_visible) {
            uint index = m_lookup_table.get_cursor_pos();
            candidate_panel_select_index(index);
        } else if (m_category_active_index > 0) {
            Gtk.ListBoxRow gtkrow = m_list_box.get_selected_row();
            EBoxRow row = gtkrow as EBoxRow;
            show_emoji_for_category(row.text);
        }
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


    private void check_action_variant_cb(GLib.SimpleAction action,
                                         GLib.Variant?     parameter) {
        m_show_emoji_variant = !action.get_state().get_boolean();
        action.set_state(new GLib.Variant.boolean(m_show_emoji_variant));
        // Redraw emoji candidate panel for m_show_emoji_variant
        if (m_candidate_panel_is_visible)
            show_candidate_panel();
    }


    public static void update_favorite_emoji_dict() {
        if (m_emoji_to_data_dict == null ||
            m_annotation_to_emojis_dict == null)
            return;

        for(int i = 0; i < m_favorites.length; i++) {
            var favorite = m_favorites[i];

            string? annotation = "";
            if (i < m_favorite_annotations.length) {
                annotation = m_favorite_annotations[i];
            }
            if (annotation == "")
                continue;
            unowned IBus.EmojiData? data =
                    m_emoji_to_data_dict.lookup(favorite);
            if (data == null) {
                GLib.SList<string> new_annotations = new GLib.SList<string>();
                new_annotations.append(annotation);
                IBus.EmojiData new_data = GLib.Object.new(
                            typeof(IBus.EmojiData),
                            "emoji", favorite.dup(),
                            "annotations", new_annotations,
                            "description", annotation.dup()
                    ) as IBus.EmojiData;
                m_emoji_to_data_dict.insert(favorite, new_data);
            } else {
                unowned GLib.SList<string> annotations = data.get_annotations();
                if (annotations.find_custom(annotation, GLib.strcmp) == null) {
                    annotations.append(annotation);
                    data.set_annotations(annotations.copy());
                }
            }
            unowned GLib.SList<string> emojis =
                    m_annotation_to_emojis_dict.lookup(annotation);
            if (emojis.find_custom(favorite, GLib.strcmp) == null) {
                emojis.append(favorite);
                m_annotation_to_emojis_dict.replace(annotation, emojis.copy());
            }
        }
    }


    public string run(string    input_context_path,
                      Gdk.Event event) {
        assert (m_loop == null);

        m_is_running = true;
        m_input_context_path = input_context_path;
        m_candidate_panel_is_visible = false;
        m_result = null;
        m_enter_notify_enable = true;

        /* Let gtk recalculate the window size. */
        resize(1, 1);

        m_entry.set_text("");

        show_category_list();
        m_entry.set_activates_default(true);
        show_all();

        /* Some window managers, e.g. MATE, GNOME, Plasma desktops,
         * does not give the keyboard focus when Emojier is lauched
         * twice with Ctrl-Shift-e via XIEvent, if present_with_time()
         * is not applied.
         * But XFCE4 desktop does not effect this bug.
         * Seems this is caused by the window manager's focus stealing
         * prevention logic:
         * https://mail.gnome.org/archives/gtk-devel-list/2017-May/msg00026.html
         */
        uint32 timestamp = event.get_time();
        present_with_time(timestamp);

        Gdk.Device pointer;
#if VALA_0_34
        Gdk.Seat seat = event.get_seat();
        if (seat == null) {
            var display = get_display();
            seat = display.get_default_seat();
        }
        pointer = seat.get_pointer();
#else
        Gdk.Device device = event.get_device();
        if (device == null) {
            var display = get_display();
            device = display.list_devices().data;
        }
        if (device.get_source() == Gdk.InputSource.KEYBOARD)
            pointer = device.get_associated_device();
        else
            pointer = device;
#endif
        pointer.get_position_double(null,
                                    out m_mouse_x,
                                    out m_mouse_y);

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
            if (key_press_escape())
                return true;
            break;
        case Gdk.Key.Return:
            key_press_enter();
            return true;
        case Gdk.Key.BackSpace:
            if (m_entry.get_text().length > 0) {
                if ((modifiers & Gdk.ModifierType.CONTROL_MASK) != 0) {
                    GLib.Signal.emit_by_name(m_entry, "delete-from-cursor",
                                             Gtk.DeleteType.WORD_ENDS, -1);
                } else {
                    GLib.Signal.emit_by_name(m_entry, "backspace");
                }
                return true;
            }
            break;
        case Gdk.Key.Delete:
        case Gdk.Key.KP_Delete:
            if (m_entry.get_text().length > 0) {
                if ((modifiers & Gdk.ModifierType.CONTROL_MASK) != 0) {
                    GLib.Signal.emit_by_name(m_entry, "delete-from-cursor",
                                             Gtk.DeleteType.WORD_ENDS, 1);
                } else {
                    GLib.Signal.emit_by_name(m_entry, "delete-from-cursor",
                                             Gtk.DeleteType.CHARS, 1);
                }
                return true;
            }
            break;
        case Gdk.Key.space:
        case Gdk.Key.KP_Space:
            if ((modifiers & Gdk.ModifierType.SHIFT_MASK) != 0) {
                if (m_entry.get_text().length > 0)
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
        case Gdk.Key.Insert:
        case Gdk.Key.KP_Insert:
            GLib.Signal.emit_by_name(m_entry, "toggle-overwrite");
            return true;
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
                if (m_entry.get_text().length > 0) {
                    GLib.Signal.emit_by_name(m_entry,
                                             "delete-from-cursor",
                                             Gtk.DeleteType.PARAGRAPH_ENDS,
                                             -1);
                    return true;
                }
                break;
            case Gdk.Key.a:
                if (m_entry.get_text().length > 0) {
                    m_entry.select_region(0, -1);
                    return true;
                }
                break;
            case Gdk.Key.x:
                if (m_entry.get_text().length > 0) {
                    GLib.Signal.emit_by_name(m_entry, "cut-clipboard");
                    return true;
                }
                break;
            case Gdk.Key.C:
            case Gdk.Key.c:
                if ((modifiers & Gdk.ModifierType.SHIFT_MASK) != 0) {
                    if (m_candidate_panel_is_visible) {
                        uint index = m_lookup_table.get_cursor_pos();
                        var text = m_lookup_table.get_candidate(index).text;
                        Gtk.Clipboard clipboard =
                                Gtk.Clipboard.get(Gdk.SELECTION_CLIPBOARD);
                        clipboard.set_text(text, -1);
                        clipboard.store();
                        return true;
                    }
                } else if (m_entry.get_text().length > 0) {
                    GLib.Signal.emit_by_name(m_entry, "copy-clipboard");
                    return true;
                }
                break;
            case Gdk.Key.v:
                GLib.Signal.emit_by_name(m_entry, "paste-clipboard");
                return true;
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


    public void present_centralize(Gdk.Event event) {
        Gtk.Allocation allocation;
        get_allocation(out allocation);
        Gdk.Rectangle monitor_area;
#if VALA_0_34
        Gdk.Display display = Gdk.Display.get_default();
        Gdk.Monitor monitor = display.get_monitor_at_window(this.get_window());
        monitor_area = monitor.get_geometry();
#else
        Gdk.Screen screen = Gdk.Screen.get_default();
        int monitor_num = screen.get_monitor_at_window(this.get_window());
        screen.get_monitor_geometry(monitor_num, out monitor_area);
#endif
        int x = (monitor_area.x + monitor_area.width - allocation.width)/2;
        int y = (monitor_area.y + monitor_area.height
                 - allocation.height)/2;
        move(x, y);

        uint32 timestamp = event.get_time();
        present_with_time(timestamp);
        m_entry.set_activates_default(true);
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


    public static void set_partial_match(bool has_partial_match) {
        m_has_partial_match = has_partial_match;
    }

    public static void set_partial_match_length(int length) {
        if (length < 1)
            return;
        m_partial_match_length = length;
    }

    public static void set_partial_match_condition(int condition) {
        if (condition < 0)
            return;
        m_partial_match_condition = condition;
    }

    public static void set_favorites(string[]? unowned_favorites,
                                     string[]? unowned_favorite_annotations) {
        m_favorites = {};
        m_favorite_annotations = {};
        for(int i = 0; i < unowned_favorites.length; i++) {
            string? favorite = unowned_favorites[i];
            // Avoid gsetting value error by manual setting
            GLib.return_if_fail(favorite != null);
            GLib.return_if_fail(favorite != "");
            m_favorites += favorite;
        }
        for(int i = 0; i < unowned_favorite_annotations.length; i++) {
            string? favorite_annotation = unowned_favorite_annotations[i];
            GLib.return_if_fail(favorite_annotation != null);
            m_favorite_annotations += favorite_annotation;
        }
        update_favorite_emoji_dict();
    }
}
