/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright(c) 2011-2015 Peng Huang <shawn.p.huang@gmail.com>
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

class ThemedRGBA {
    public Gdk.RGBA *normal_fg { get; set; }
    public Gdk.RGBA *normal_bg { get; set; }
    public Gdk.RGBA *selected_fg { get; set; }
    public Gdk.RGBA *selected_bg { get; set; }

    private Gtk.StyleContext m_style_context;

    public ThemedRGBA(Gtk.Widget widget) {
        this.normal_fg = null;
        this.normal_bg = null;
        this.selected_fg = null;
        this.selected_bg = null;

        /* Use the color of Gtk.TextView instead of Gtk.Label
         * because the selected label "color" is not configured
         * in "Adwaita" theme and the selected label "background-color"
         * is not configured in "Maia" theme.
         * https://github.com/ibus/ibus/issues/1871
         */
        Gtk.WidgetPath widget_path = new Gtk.WidgetPath();
        widget_path.append_type(typeof(Gtk.TextView));
        m_style_context = new Gtk.StyleContext();
        m_style_context.set_path(widget_path);
        m_style_context.add_class(Gtk.STYLE_CLASS_VIEW);

        /* "-gtk-secondary-caret-color" value is different
         * if the parent widget is set in "Menta" theme.
         */
        m_style_context.set_parent(widget.get_style_context());

        get_rgba();

        m_style_context.changed.connect(() => { get_rgba(); });
    }

    ~ThemedRGBA() {
        reset_rgba();
    }

    private void reset_rgba() {
        if (this.normal_fg != null) {
            this.normal_fg.free();
            this.normal_fg = null;
        }
        if (this.normal_bg != null) {
            this.normal_bg.free();
            this.normal_bg = null;
        }
        if (this.selected_fg != null) {
            this.selected_fg.free();
            this.selected_fg = null;
        }
        if (this.selected_bg != null) {
            this.selected_bg.free();
            this.selected_bg = null;
        }
    }

    private void get_rgba() {
        reset_rgba();
        Gdk.RGBA *normal_fg = null;
        Gdk.RGBA *normal_bg = null;
        Gdk.RGBA *selected_fg = null;
        Gdk.RGBA *selected_bg = null;
        m_style_context.get(Gtk.StateFlags.NORMAL,
                            "color",
                            out normal_fg);
        m_style_context.get(Gtk.StateFlags.SELECTED,
                            "color",
                            out selected_fg);

        string bg_prop = "background-color";
        m_style_context.get(Gtk.StateFlags.NORMAL,
                            bg_prop,
                            out normal_bg);
        m_style_context.get(Gtk.StateFlags.SELECTED,
                            bg_prop,
                            out selected_bg);
        if (normal_bg.red   == selected_bg.red &&
            normal_bg.green == selected_bg.green &&
            normal_bg.blue  == selected_bg.blue &&
            normal_bg.alpha == selected_bg.alpha) {
            normal_bg.free();
            normal_bg = null;
            normal_bg.free();
            normal_bg = null;
            bg_prop = "-gtk-secondary-caret-color";
            m_style_context.get(Gtk.StateFlags.NORMAL,
                                bg_prop,
                                out normal_bg);
            m_style_context.get(Gtk.StateFlags.SELECTED,
                                bg_prop,
                                out selected_bg);
        }
        this.normal_fg   = normal_fg;
        this.normal_bg   = normal_bg;
        this.selected_fg = selected_fg;
        this.selected_bg = selected_bg;
    }
}

class CandidateArea : Gtk.Box {
    private bool m_vertical;
    private Gtk.Label[] m_labels;
    private Gtk.Label[] m_candidates;
    private Gtk.Widget[] m_widgets;

    private IBus.Text[] m_ibus_candidates;
    private uint m_focus_candidate;
    private bool m_show_cursor;
    private ThemedRGBA m_rgba;

    private const string LABELS[] = {
        "1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.",
        "9.", "0.", "a.", "b.", "c.", "d.", "e.", "f."
    };

    private const string PREV_PAGE_ICONS[] = {
        "go-previous",
        "go-up"
    };

    private const string NEXT_PAGE_ICONS[] = {
        "go-next",
        "go-down"
    };

    public signal void candidate_clicked(uint index, uint button, uint state);
    public signal void page_up();
    public signal void page_down();
    public signal void cursor_up();
    public signal void cursor_down();

    public CandidateArea(bool vertical) {
        GLib.Object();
        set_vertical(vertical, true);
        m_rgba = new ThemedRGBA(this);
    }

    public bool candidate_scrolled(Gdk.EventScroll event) {
        switch (event.direction) {
        case Gdk.ScrollDirection.UP:
            cursor_up();
            break;
        case Gdk.ScrollDirection.DOWN:
            cursor_down();
            break;
        }
        return true;
    }

    public bool get_vertical() {
        return m_vertical;
    }

    public void set_vertical(bool vertical, bool force = false) {
        if (!force && m_vertical == vertical)
            return;
        m_vertical = vertical;
        orientation = vertical ?
            Gtk.Orientation.VERTICAL :
            Gtk.Orientation.HORIZONTAL;
        recreate_ui();

        if (m_ibus_candidates.length > 0) {
            // Workaround a vala issue
            // https://bugzilla.gnome.org/show_bug.cgi?id=661130
            set_candidates((owned)m_ibus_candidates,
                           m_focus_candidate,
                           m_show_cursor);
            show_all();
        }
    }

    public void set_labels(IBus.Text[] labels) {
        int i;
        for (i = 0; i < int.min(16, labels.length); i++)
            m_labels[i].set_text(labels[i].get_text());
        for (; i < 16; i++)
            m_labels[i].set_text(LABELS[i]);
    }

    public void set_candidates(IBus.Text[] candidates,
                               uint focus_candidate = 0,
                               bool show_cursor = true) {
        m_ibus_candidates = candidates;
        m_focus_candidate = focus_candidate;
        m_show_cursor = show_cursor;

        assert(candidates.length < 16);
        for (int i = 0 ; i < 16 ; i++) {
            Gtk.Label label = m_candidates[i];
            bool visible = false;
            if (i < candidates.length) {
                Pango.AttrList attrs = get_pango_attr_list_from_ibus_text(candidates[i]);
                if (i == focus_candidate && show_cursor) {
                    Pango.Attribute pango_attr = Pango.attr_foreground_new(
                            (uint16)(m_rgba.selected_fg.red * uint16.MAX),
                            (uint16)(m_rgba.selected_fg.green * uint16.MAX),
                            (uint16)(m_rgba.selected_fg.blue * uint16.MAX));
                    pango_attr.start_index = 0;
                    pango_attr.end_index = candidates[i].get_text().length;
                    attrs.insert((owned)pango_attr);

                    pango_attr = Pango.attr_background_new(
                           (uint16)(m_rgba.selected_bg.red * uint16.MAX),
                           (uint16)(m_rgba.selected_bg.green * uint16.MAX),
                           (uint16)(m_rgba.selected_bg.blue * uint16.MAX));
                    pango_attr.start_index = 0;
                    pango_attr.end_index = candidates[i].get_text().length;
                    attrs.insert((owned)pango_attr);
                }
                label.set_text(candidates[i].get_text());
                label.set_attributes(attrs);
                visible = true;
            } else {
                label.set_text("");
                label.set_attributes(new Pango.AttrList());
            }
            if (m_vertical) {
                m_widgets[i * 2].set_visible(visible);
                m_widgets[i * 2 +1].set_visible(visible);
            } else {
                m_widgets[i].set_visible(visible);
            }
        }
    }

    private void recreate_ui() {
        foreach (Gtk.Widget w in get_children()) {
            w.destroy();
        }

        Gtk.Button prev_button = new Gtk.Button();
        prev_button.clicked.connect((b) => page_up());
        prev_button.set_image(new Gtk.Image.from_icon_name(
                                  PREV_PAGE_ICONS[orientation],
                                  Gtk.IconSize.MENU));
        prev_button.set_relief(Gtk.ReliefStyle.NONE);

        Gtk.Button next_button = new Gtk.Button();
        next_button.clicked.connect((b) => page_down());
        next_button.set_image(new Gtk.Image.from_icon_name(
                                  NEXT_PAGE_ICONS[orientation],
                                  Gtk.IconSize.MENU));
        next_button.set_relief(Gtk.ReliefStyle.NONE);

        if (m_vertical) {
            Gtk.EventBox container_ebox = new Gtk.EventBox();
            container_ebox.add_events(Gdk.EventMask.SCROLL_MASK);
            container_ebox.scroll_event.connect(candidate_scrolled);
            add(container_ebox);

            Gtk.Box vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
            container_ebox.add(vbox);

            // Add Candidates
            Gtk.Box candidates_hbox = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            vbox.pack_start(candidates_hbox, false, false, 0);
            Gtk.Box labels_vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
            labels_vbox.set_homogeneous(true);
            Gtk.Box candidates_vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
            candidates_vbox.set_homogeneous(true);
            candidates_hbox.pack_start(labels_vbox, false, false, 4);
            candidates_hbox.pack_start(new VSeparator(), false, false, 0);
            candidates_hbox.pack_start(candidates_vbox, true, true, 4);

            // Add HSeparator
            vbox.pack_start(new HSeparator(), false, false, 0);

            // Add buttons
            Gtk.Box buttons_hbox = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            Gtk.Label state_label = new Gtk.Label(null);
            state_label.set_size_request(20, -1);
            buttons_hbox.pack_start(state_label, true, true, 0);
            buttons_hbox.pack_start(prev_button, false, false, 0);
            buttons_hbox.pack_start(next_button, false, false, 0);
            vbox.pack_start(buttons_hbox, false, false, 0);

            m_labels = {};
            m_candidates = {};
            m_widgets = {};
            for (int i = 0; i < 16; i++) {
                Gtk.Label label = new Gtk.Label(LABELS[i]);
                label.set_halign(Gtk.Align.START);
                label.set_valign(Gtk.Align.CENTER);
                label.show();
                m_labels += label;

                Gtk.Label candidate = new Gtk.Label("test");
                candidate.set_halign(Gtk.Align.START);
                candidate.set_valign(Gtk.Align.CENTER);
                candidate.show();
                m_candidates += candidate;

                /* Use Gtk.Widget.set_margin_start() since gtk 3.12 */
                label.set_padding(8, 0);
                candidate.set_padding(8, 0);

                // Make a copy of i to workaround a bug in vala.
                // https://bugzilla.gnome.org/show_bug.cgi?id=628336
                int index = i;
                Gtk.EventBox label_ebox = new Gtk.EventBox();
                label_ebox.set_no_show_all(true);
                label_ebox.button_press_event.connect((w, e) => {
                    candidate_clicked(i, e.button, e.state);
                    return true;
                });
                label_ebox.add(label);
                labels_vbox.pack_start(label_ebox, false, false, 2);
                m_widgets += label_ebox;

                Gtk.EventBox candidate_ebox = new Gtk.EventBox();
                candidate_ebox.set_no_show_all(true);
                candidate_ebox.button_press_event.connect((w, e) => {
                    candidate_clicked(index, e.button, e.state);
                    return true;
                });
                candidate_ebox.add(candidate);
                candidates_vbox.pack_start(candidate_ebox, false, false, 2);
                m_widgets += candidate_ebox;
            }
        } else {
            Gtk.EventBox container_ebox = new Gtk.EventBox();
            container_ebox.add_events(Gdk.EventMask.SCROLL_MASK);
            container_ebox.scroll_event.connect(candidate_scrolled);
            add(container_ebox);

            Gtk.Box hbox = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            container_ebox.add(hbox);

            m_labels = {};
            m_candidates = {};
            m_widgets = {};
            for (int i = 0; i < 16; i++) {
                Gtk.Label label = new Gtk.Label(LABELS[i]);
                label.set_halign(Gtk.Align.START);
                label.set_valign(Gtk.Align.CENTER);
                label.show();
                m_labels += label;

                Gtk.Label candidate = new Gtk.Label("test");
                candidate.set_halign(Gtk.Align.START);
                candidate.set_valign(Gtk.Align.CENTER);
                candidate.show();
                m_candidates += candidate;

                Gtk.Box candidate_hbox = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
                candidate_hbox.show();
                candidate_hbox.pack_start(label, false, false, 2);
                candidate_hbox.pack_start(candidate, false, false, 2);

                // Make a copy of i to workaround a bug in vala.
                // https://bugzilla.gnome.org/show_bug.cgi?id=628336
                int index = i;
                Gtk.EventBox ebox = new Gtk.EventBox();
                ebox.set_no_show_all(true);
                ebox.button_press_event.connect((w, e) => {
                    candidate_clicked(index, e.button, e.state);
                    return true;
                });
                ebox.add(candidate_hbox);
                hbox.pack_start(ebox, false, false, 4);
                m_widgets += ebox;
            }
            hbox.pack_start(new VSeparator(), false, false, 0);
            hbox.pack_start(prev_button, false, false, 0);
            hbox.pack_start(next_button, false, false, 0);
        }
    }
}

